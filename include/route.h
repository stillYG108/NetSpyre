#ifndef ROUTE_H
#define ROUTE_H

/*
    Function to send a request to the kernel to get the route information

    @param sock_fd : file descriptor of the netlink socket
*/
void send_req_route_info_helper(int sock_fd);

/*
    Function to recieve the route information from the kernel

    @param sock_fd : file descriptor of the netlink socket
*/
void recieve_route_info_helper(int sock_fd);

/*
    Function to print the route information

    @param nlh : netlink message header
*/
void print_route_info_helper(struct nlmsghdr *nlh);

#endif