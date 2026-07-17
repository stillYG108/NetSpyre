#ifndef ADDR_H
#define ADDR_H
/*
    Function to send a request to the kernel to get the address information

    @param sock_fd : file descriptor of the netlink socket
*/
void send_req_addr_info_helper(int sock_fd);

/*
    Function to recieve the address information from the kernel

    @param sock_fd : file descriptor of the netlink socket
*/
void recieve_addr_info_helper(int sock_fd);

/*
    Function to print the address information

    @param nlh : netlink message header
*/
void print_addr_info_helper(struct nlmsghdr *nlh);

#endif // ADDR_H