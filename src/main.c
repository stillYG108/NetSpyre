#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <string.h>


#include "../include/netlink.h"
#include "../include/link.h"
#include "../include/addr.h"
#include "../include/route.h"
#include "../include/neigh.h"


int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: sudo %s [link | addr | route | neigh]\n", argv[0]);
        exit(EXIT_FAILURE);
    }



    // 1. Create the Netlink Socket for routing subsystem
    int sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    
    if (sock_fd < 0) {
        perror("Socket creation failed. Did you run as root with sudo?");
        exit(EXIT_FAILURE);
    }
    printf("Netlink socket successfully opened with fd: %d\n", sock_fd);

    // 2. Bind the socket
    struct sockaddr_nl la;
    memset(&la, 0, sizeof(la));
    la.nl_family = AF_NETLINK;
    la.nl_pid = getpid(); // Local port ID (typically our PID)
    la.nl_groups = 0;     // 0 for unicast queries

    if (bind(sock_fd, (struct sockaddr *)&la, sizeof(la)) < 0) {
        perror("Bind failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("Netlink socket bound successfully.\n");

    // 3. Dispatch requests based on user CLI input
    if (strcmp(argv[1], "link") == 0) {
        send_req_link_info_helper(sock_fd);
        recieve_link_info(sock_fd);
    } 
    else if (strcmp(argv[1], "addr") == 0) {
        send_req_addr_info_helper(sock_fd);
        recieve_addr_info_helper(sock_fd);
    } 
    else if (strcmp(argv[1], "route") == 0) {
        send_req_route_info_helper(sock_fd);
        recieve_route_info_helper(sock_fd);
    } 
    else if (strcmp(argv[1], "neigh") == 0) {
        send_req_neigh_info_helper(sock_fd);
        recieve_neigh_info_helper(sock_fd);
    } 
    else {
        fprintf(stderr, "Error: Unknown command '%s'\n", argv[1]);
        fprintf(stderr, "Usage: sudo %s [link | addr | route | neigh]\n", argv[0]);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    close(sock_fd);
    return 0;
}
