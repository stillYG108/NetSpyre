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
#include "../include/route.h"


int main() {
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


/*
    // 3. Send a request to get the interface (link) list
    struct rtgenmsg rt_hdr;
    rt_hdr.rtgen_family = AF_PACKET; // AF_PACKET/AF_UNSPEC gets all link interfaces

    if (netlink_send_helper(sock_fd, RTM_GETLINK, NLM_F_REQUEST | NLM_F_DUMP, 1, &rt_hdr, sizeof(rt_hdr)) < 0) {
        fprintf(stderr, "Failed to send link request\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("Link list request sent successfully.\n");
*/


/*
// 4. Receive and parse multipart responses in a loop
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

            // Process payload data here (e.g. NLMSG_DATA(nlh))
            printf("Received message type: %d, length: %d\n", nlh->nlmsg_type, nlh->nlmsg_len);
            
            nlh = NLMSG_NEXT(nlh, bytes_received);
            }
            }
            
            */

    send_req_link_info_helper(sock_fd);
    recieve_link_info(sock_fd);

    send_req_addr_info_helper(sock_fd);
    recieve_addr_info_helper(sock_fd);

    send_req_route_info_helper(sock_fd);
    recieve_route_info_helper(sock_fd);
    
    close(sock_fd);
    return 0;
}
