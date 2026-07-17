#ifndef LINK_H
#define LINK_H

/**
 * Sends a request to retrieve link (interface) information.
 * 
 * @param sock_fd The open Netlink socket descriptor.
 */
void send_req_link_info_helper(int sock_fd);

/**
 * Receives the link information from the kernel and prints it
 * 
 * @param sock_fd The open Netlink socket descriptor.
*/
void recieve_link_info(int sock_fd);

/**
 * Parse the recieved link information from the kernel
 *
 * @param nlh The Netlink message header to parse
*/
void print_link_info_helper(struct nlmsghdr *nlh);

#endif // LINK_H
