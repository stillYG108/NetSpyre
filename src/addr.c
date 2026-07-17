#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h> // Essential for Netlink & RTNETLINK structures
#include <string.h>
#include <stdint.h>
#include "../include/netlink.h"
#include <net/if.h>
#include "../include/addr.h"
#include "../include/utils.h"
#include "../include/link.h"
#include <arpa/inet.h>

void send_req_addr_info_helper(int sock_fd){
    struct ifaddrmsg ad_hdr;
    memset(&ad_hdr , 0, sizeof(ad_hdr));
    ad_hdr.ifa_family = AF_UNSPEC;

    if(netlink_send_helper(sock_fd,RTM_GETADDR, NLM_F_REQUEST | NLM_F_DUMP,1,&ad_hdr,sizeof(ad_hdr))<0){
        fprintf(stderr, "Failed to send addr request\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
}

void recieve_addr_info_helper(int sock_fd){
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

            if (nlh->nlmsg_type == RTM_NEWADDR) {
                print_addr_info_helper(nlh);
            }

            nlh = NLMSG_NEXT(nlh, bytes_received);
        }
    }
}

void print_addr_info_helper(struct nlmsghdr *nlh) {
    struct ifaddrmsg *ifm = (struct ifaddrmsg *)NLMSG_DATA(nlh);
    
    char ifname[IFNAMSIZ] = "unknown";
    if_indextoname(ifm->ifa_index, ifname);

    struct rtattr *ifa_attr_ptr = IFA_RTA(ifm);
    int ifa_attr_len = IFA_PAYLOAD(nlh);

    char ip_str[INET6_ADDRSTRLEN] = "";
    int has_addr = 0;

    while (RTA_OK(ifa_attr_ptr, ifa_attr_len)) {
        if (ifa_attr_ptr->rta_type == IFA_LOCAL || ifa_attr_ptr->rta_type == IFA_ADDRESS) {
            if (ifm->ifa_family == AF_INET) {
                struct in_addr *addr = (struct in_addr *)RTA_DATA(ifa_attr_ptr);
                if (inet_ntop(AF_INET, addr, ip_str, sizeof(ip_str)) != NULL) {
                    has_addr = 1;
                }
            } else if (ifm->ifa_family == AF_INET6) {
                struct in6_addr *addr6 = (struct in6_addr *)RTA_DATA(ifa_attr_ptr);
                if (inet_ntop(AF_INET6, addr6, ip_str, sizeof(ip_str)) != NULL) {
                    has_addr = 1;
                }
            }
            
            if (ifa_attr_ptr->rta_type == IFA_LOCAL) {
                break;
            }
        }
        ifa_attr_ptr = RTA_NEXT(ifa_attr_ptr, ifa_attr_len);
    }

    if (has_addr) {
        printf("\033[1;35m[Address] Interface: %s\033[0m\n", ifname);
        
        if (ifm->ifa_family == AF_INET) {
            printf("    Family:     IPv4 (INET)\n");
            printf("    Address:    %s/%d\n", ip_str, ifm->ifa_prefixlen);
            
            uint32_t mask = (ifm->ifa_prefixlen == 0) ? 0 : (~0U << (32 - ifm->ifa_prefixlen));
            struct in_addr mask_addr;
            mask_addr.s_addr = htonl(mask);
            char mask_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &mask_addr, mask_str, sizeof(mask_str));
            printf("    Netmask:    %s\n", mask_str);
        } else if (ifm->ifa_family == AF_INET6) {
            printf("    Family:     IPv6 (INET6)\n");
            printf("    Address:    %s/%d\n", ip_str, ifm->ifa_prefixlen);
        }
        
        const char *scope_str = "unknown";
        switch (ifm->ifa_scope) {
            case RT_SCOPE_UNIVERSE: scope_str = "global"; break;
            case RT_SCOPE_SITE:     scope_str = "site"; break;
            case RT_SCOPE_LINK:     scope_str = "link"; break;
            case RT_SCOPE_HOST:     scope_str = "host"; break;
            case RT_SCOPE_NOWHERE:  scope_str = "nowhere"; break;
        }
        printf("    Scope:      %s\n", scope_str);
        printf("\033[1;30m-----------------------------------------------------\033[0m\n");
    }
}
