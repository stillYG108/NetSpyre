#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h> // Essential for Netlink & RTNETLINK structures
#include <string.h>
#include <stdint.h>
#include "../include/netlink.h"
#include "../include/link.h"
#include <net/if.h>

// Helper to convert raw bytes into human-readable format
void format_bytes(uint64_t bytes, char *out, size_t out_len) {
    double kb = bytes / 1024.0;
    double mb = kb / 1024.0;
    double gb = mb / 1024.0;
    
    if (gb >= 1.0) {
        snprintf(out, out_len, "%.2f GiB", gb);
    } else if (mb >= 1.0) {
        snprintf(out, out_len, "%.2f MiB", mb);
    } else if (kb >= 1.0) {
        snprintf(out, out_len, "%.2f KiB", kb);
    } else {
        snprintf(out, out_len, "%lu Bytes", (unsigned long)bytes);
    }
}

/*
    This function is used to send a request to the kernel to get the link 
*/
void send_req_link_info_helper(int sock_fd) {
    struct ifinfomsg if_hdr;
    memset(&if_hdr, 0, sizeof(if_hdr)); // Zero out garbage memory
    if_hdr.ifi_family = AF_UNSPEC;      // AF_PACKET/AF_UNSPEC gets all link interfaces

    if (netlink_send_helper(sock_fd, RTM_GETLINK, NLM_F_REQUEST | NLM_F_DUMP, 1, &if_hdr, sizeof(if_hdr)) < 0) {
        fprintf(stderr, "Failed to send link request\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }   
}
    
/*
    This function is used to recieve a link from the kernel 
*/
void recieve_link_info(int sock_fd) {
    int done = 0;
    while (!done) {
        char my_buffer[BUFFER_SIZE];
        ssize_t bytes_received = netlink_receive_helper(sock_fd, my_buffer, sizeof(my_buffer));

        if (bytes_received <= 0) {
            break; // Error or socket closed
        }

        struct nlmsghdr *nlh = (struct nlmsghdr *)my_buffer;

        while (NLMSG_OK(nlh, bytes_received)) {
            if (nlh->nlmsg_type == NLMSG_DONE) {
                done = 1;
                break;
            }
            if (nlh->nlmsg_type == NLMSG_ERROR) {
                done = 1;
                break;
            }

            if (nlh->nlmsg_type == RTM_NEWLINK) {
                print_link_info_helper(nlh);
            }

            nlh = NLMSG_NEXT(nlh, bytes_received);
        }
    }
}

void print_link_info_helper(struct nlmsghdr *nlh) {
    // 1. Get pointer to the ifinfomsg header
    struct ifinfomsg *ifm = (struct ifinfomsg *)NLMSG_DATA(nlh);

    char ifname[IFNAMSIZ] = "unknown";
    uint32_t mtu = 0;
    unsigned char mac[6] = {0};
    int has_mac = 0;
    struct rtnl_link_stats64 stats;
    int has_stats = 0;
    
    // 2. Safely skip the ifinfomsg header to reach the first rtattr
    struct rtattr *rta = IFLA_RTA(ifm);
    
    // 3. Get the total byte length of all trailing attributes
    int rta_len = IFLA_PAYLOAD(nlh);

    // 4. Loop through the raw byte payload attribute by attribute
    while (RTA_OK(rta, rta_len)) {
        switch (rta->rta_type) {
            case IFLA_IFNAME:
                snprintf(ifname, sizeof(ifname), "%s", (char *)RTA_DATA(rta));
                break;
                
            case IFLA_MTU:
                mtu = *(uint32_t *)RTA_DATA(rta);
                break;
                
            case IFLA_ADDRESS:{
                int mac_len = RTA_PAYLOAD(rta); // Get actual length of the address
                if (mac_len == 6) { // Standard Ethernet MAC
                    memcpy(mac, RTA_DATA(rta), 6);
                    has_mac = 1;
                }
                break;
            }
            case IFLA_STATS64:{
                memcpy(&stats, RTA_DATA(rta), sizeof(stats));
                has_stats = 1;
                break; // Found our statistics payload
            }
        }
        
        // Advance to the next attribute (handles internal alignment padding)
        rta = RTA_NEXT(rta, rta_len);
    }

    // Print a structured, colored output
    printf("\033[1;36m[%d] Interface: %s\033[0m\n", ifm->ifi_index, ifname);
    
    const char *state = (ifm->ifi_flags & IFF_UP) ? "\033[1;32mUP\033[0m" : "\033[1;31mDOWN\033[0m";
    printf("    State:      %s", state);
    if (ifm->ifi_flags & IFF_LOOPBACK) {
        printf(" (LOOPBACK)");
    }
    printf("\n");
    printf("    MTU:        %u\n", mtu);

    if (has_mac) {
        printf("    MAC Addr:   %02x:%02x:%02x:%02x:%02x:%02x\n",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } else {
        printf("    MAC Addr:   none\n");
    }

    if (has_stats) {
        char rx_size[32], tx_size[32];
        format_bytes(stats.rx_bytes, rx_size, sizeof(rx_size));
        format_bytes(stats.tx_bytes, tx_size, sizeof(tx_size));

        printf("    Statistics:\n");
        printf("      RX: %-12s | %llu Packets\n", rx_size, (unsigned long long)stats.rx_packets);
        printf("      TX: %-12s | %llu Packets\n", tx_size, (unsigned long long)stats.tx_packets);
    }
    printf("\033[1;30m-----------------------------------------------------\033[0m\n");
}
