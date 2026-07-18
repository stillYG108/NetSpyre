#ifndef NEIGH_H
#define NEIGH_H

#include <sys/socket.h>
#include <linux/rtnetlink.h>

#ifndef NDA_RTA
#define NDA_RTA(r) ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ndmsg))))
#endif

#ifndef NDA_PAYLOAD
#define NDA_PAYLOAD(n) NLMSG_PAYLOAD(n, sizeof(struct ndmsg))
#endif

void send_req_neigh_info_helper(int sock_fd);
void recieve_neigh_info_helper(int sock_fd);
void print_neigh_info_helper(struct nlmsghdr *nlh);

#endif