#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h> // Essential for Netlink & RTNETLINK structures
#include <string.h>
#include <stdint.h>
#include <net/if.h>
#include "../include/netlink.h"
#include "../include/route.h"
#include <arpa/inet.h>



void send_req_route_info_helper(int sock_fd){
    struct rtmsg rt_hdr;
    memset(&rt_hdr, 0, sizeof(rt_hdr));
    rt_hdr.rtm_family = AF_UNSPEC;

    if(netlink_send_helper(sock_fd,RTM_GETROUTE, NLM_F_REQUEST | NLM_F_DUMP,1,&rt_hdr,sizeof(rt_hdr))<0){
        fprintf(stderr, "Failed to send route request\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
}

void recieve_route_info_helper(int sock_fd){
    int done = 0;
    while(!done){
        char buffer[BUFFER_SIZE];
        ssize_t bytes_received = netlink_receive_helper(sock_fd, buffer, sizeof(buffer));
        if (bytes_received <= 0) {
            break;
        }
        struct nlmsghdr *nlh = (struct nlmsghdr *)buffer;
        while (NLMSG_OK(nlh, bytes_received)) {
            if (nlh->nlmsg_type == NLMSG_DONE) {
                done = 1;
                break;
            }
            if (nlh->nlmsg_type == NLMSG_ERROR) {
                done = 1;
                break;
            }

            if (nlh->nlmsg_type == RTM_NEWROUTE) {
                print_route_info_helper(nlh);
            }

            nlh = NLMSG_NEXT(nlh, bytes_received);
        }
    }
}


void print_route_info_helper(struct nlmsghdr *nlh){
    struct rtmsg *rtm = (struct rtmsg *)NLMSG_DATA(nlh);

    char preferred_src[INET6_ADDRSTRLEN] = "none";
    char dest[INET6_ADDRSTRLEN] = "none";
    char gateway[INET6_ADDRSTRLEN] = "none";
    char ifname[IFNAMSIZ] = "unknown";

    struct rtattr *rta = RTM_RTA(rtm);
    int rta_len = RTM_PAYLOAD(nlh);
    
    while(RTA_OK(rta, rta_len)){
        switch(rta->rta_type){
            case RTA_PREFSRC:{
                inet_ntop(rtm->rtm_family, RTA_DATA(rta), preferred_src, sizeof(preferred_src));
                break;
            }
            case RTA_DST:{
                inet_ntop(rtm->rtm_family, RTA_DATA(rta), dest, sizeof(dest));
                break;
            }
            case RTA_GATEWAY:{
                inet_ntop(rtm->rtm_family, RTA_DATA(rta), gateway, sizeof(gateway));
                break;
            }
            case RTA_OIF:{
                int if_index = *(int *)RTA_DATA(rta);
                if_indextoname(if_index, ifname);
                break;
            }
        }
        rta = RTA_NEXT(rta, rta_len);
    }

    if (rtm->rtm_dst_len == 0) {
        printf("[Route] Destination: default\n");
    } else {
        printf("[Route] Destination: %s/%d\n", dest, rtm->rtm_dst_len);
    }

    printf("    Family:     %s\n", (rtm->rtm_family == AF_INET) ? "IPv4 (INET)" : "IPv6 (INET6)");
    printf("    Gateway:    %s\n", (strcmp(gateway, "none") == 0) ? "None" : gateway);
    printf("    Interface:  %s\n", ifname);
    printf("    Source IP:  %s\n", (strcmp(preferred_src, "none") == 0) ? "None" : preferred_src);
    printf("-----------------------------------------------------\n");


}