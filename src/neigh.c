#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h> // Essential for Netlink & RTNETLINK structures
#include <string.h>
#include <stdint.h>
#include "../include/netlink.h"
#include "../include/neigh.h"
#include <net/if.h>
#include <arpa/inet.h>


void send_req_neigh_info_helper(int sock_fd) {
    struct ndmsg nd_hdr;
    memset(&nd_hdr, 0, sizeof(nd_hdr)); // Zero out garbage memory
    nd_hdr.ndm_family = AF_UNSPEC;      // AF_PACKET/AF_UNSPEC gets all link interfaces

    if (netlink_send_helper(sock_fd, RTM_GETNEIGH, NLM_F_REQUEST | NLM_F_DUMP, 1, &nd_hdr, sizeof(nd_hdr)) < 0) {
        fprintf(stderr, "Failed to send neigh request\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }   
}

void recieve_neigh_info_helper(int sock_fd){
    int done = 0;
    while(!done){
        char buffer[BUFFER_SIZE];
        
        ssize_t bytes_received = netlink_receive_helper(sock_fd, buffer, BUFFER_SIZE);
        
        if(bytes_received <=0){
            break;
        }

        struct nlmsghdr *nlh = (struct nlmsghdr *)buffer;

        while(NLMSG_OK(nlh, bytes_received)){
            if(nlh->nlmsg_type == NLMSG_DONE){
                done = 1;
                break;
            }
            if(nlh->nlmsg_type == NLMSG_ERROR){
                done = 1;
                break;
            }

            if(nlh->nlmsg_type == RTM_NEWNEIGH){
                print_neigh_info_helper(nlh);
            }
            nlh = NLMSG_NEXT(nlh, bytes_received);
        }
    }
}

void print_neigh_info_helper(struct nlmsghdr *nlh){
    struct ndmsg *nd_msg = (struct ndmsg *)NLMSG_DATA(nlh);
    struct rtattr *rta = NDA_RTA(nd_msg);

    char ip[INET6_ADDRSTRLEN] = "none";
    char hw_addr[18] = "";
    char ifname[IFNAMSIZ] = "unknown";
    
    int rta_len = NDA_PAYLOAD(nlh);

    if_indextoname(nd_msg->ndm_ifindex, ifname);

    const char *state_str = "UNKNOWN";
    if (nd_msg->ndm_state & NUD_REACHABLE) state_str = "\033[1;32mREACHABLE\033[0m";
    else if (nd_msg->ndm_state & NUD_STALE) state_str = "\033[1;33mSTALE\033[0m";
    else if (nd_msg->ndm_state & NUD_DELAY) state_str = "\033[1;33mDELAY\033[0m";
    else if (nd_msg->ndm_state & NUD_PROBE) state_str = "\033[1;33mPROBE\033[0m";
    else if (nd_msg->ndm_state & NUD_FAILED) state_str = "\033[1;31mFAILED\033[0m";
    else if (nd_msg->ndm_state & NUD_NOARP) state_str = "\033[1;30mNOARP\033[0m";
    else if (nd_msg->ndm_state & NUD_PERMANENT) state_str = "\033[1;35mPERMANENT\033[0m";
    else if (nd_msg->ndm_state & NUD_INCOMPLETE) state_str = "\033[1;31mINCOMPLETE\033[0m";


    while(RTA_OK(rta, rta_len)){
        switch(rta->rta_type){  
            case NDA_DST:
                if(nd_msg->ndm_family == AF_INET){
                    inet_ntop(AF_INET, RTA_DATA(rta), ip, sizeof(ip));
                }else if(nd_msg->ndm_family == AF_INET6){
                    inet_ntop(AF_INET6, RTA_DATA(rta), ip, sizeof(ip));
                }
                break;
            case NDA_LLADDR:
                snprintf(hw_addr, sizeof(hw_addr), "%02x:%02x:%02x:%02x:%02x:%02x",
                         ((unsigned char*)RTA_DATA(rta))[0], 
                         ((unsigned char*)RTA_DATA(rta))[1], 
                         ((unsigned char*)RTA_DATA(rta))[2], 
                         ((unsigned char*)RTA_DATA(rta))[3], 
                         ((unsigned char*)RTA_DATA(rta))[4], 
                         ((unsigned char*)RTA_DATA(rta))[5]);
                break;
        }
        rta = RTA_NEXT(rta, rta_len);
    }

    // 5. Print the formatted neighbor details
    printf("\033[1;34m[Neighbor] IP:      %s\033[0m\n", ip);
    printf("    Interface:  %s\n", ifname);
    printf("    MAC Addr:   %s\n", (strlen(hw_addr) == 0) ? "None" : hw_addr);
    printf("    State:      %s\n", state_str);
    printf("\033[1;30m-----------------------------------------------------\033[0m\n");
}