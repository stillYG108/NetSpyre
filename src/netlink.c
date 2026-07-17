#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h> // Essential for Netlink & RTNETLINK structures
#include <string.h>
#include <stdint.h>
#include "../include/netlink.h"


int netlink_send_helper(int sock_fd, uint16_t msg_type, uint16_t flags, 
                        uint32_t seq_num, const void *payload, size_t payload_len){
    
    // 1. Calculate total memory needed (Header + Payload aligned)
    size_t total_len = NLMSG_SPACE(payload_len);
    
    // 2. Allocate memory for the full Netlink message
    struct nlmsghdr *nlh = (struct nlmsghdr *)malloc(total_len);
    if (!nlh) {
        perror("Memory allocation failed");
        return -1;
    }
    memset(nlh, 0, total_len);

    // 3. Populate Netlink header fields
    nlh->nlmsg_len = NLMSG_LENGTH(payload_len); // Length including header
    nlh->nlmsg_type = msg_type;                // e.g., RTM_NEWLINK, RTM_GETROUTE
    nlh->nlmsg_flags = flags;                  // e.g., NLM_F_REQUEST | NLM_F_ACK
    nlh->nlmsg_seq = seq_num;                  // Sequence number for tracking
    nlh->nlmsg_pid = getpid();                 // Sending process PID

    // 4. Copy the payload into the message body
    if (payload && payload_len > 0) {
        memcpy(NLMSG_DATA(nlh), payload, payload_len);
    }

    // 5. Define destination address (Kernel)
    struct sockaddr_nl dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;    // 0 targets the kernel
    dest_addr.nl_groups = 0; // Unicast

    // 6. Send the message using sendto()
    ssize_t sent_bytes = sendto(sock_fd, nlh, nlh->nlmsg_len, 0,
                                (struct sockaddr *)&dest_addr, sizeof(dest_addr));

    // 7. Clean up memory
    free(nlh);

    if (sent_bytes < 0) {
        perror("Netlink sendto failed");
        return -1;
    }

    return 0;
}


ssize_t netlink_receive_helper(int sock_fd, char *out_buffer, size_t buffer_len) {
    if (sock_fd < 0 || out_buffer == NULL || buffer_len < sizeof(struct nlmsghdr)) {
        return -1;
    }

    // Clear the destination buffer
    memset(out_buffer, 0, buffer_len);

    // Block and read raw bytes from the Netlink socket
    ssize_t read_bytes = recv(sock_fd, out_buffer, buffer_len, 0);
    if (read_bytes < 0) {
        perror("Netlink recv failed");
        return -1;
    }

    // Cast the start of the buffer to the Netlink message header
    struct nlmsghdr *nlh = (struct nlmsghdr *)out_buffer;

    // Validate the basic sanity of the received buffer data
    if (!NLMSG_OK(nlh, read_bytes)) {
        fprintf(stderr, "Error: Received truncated or malformed Netlink message.\n");
        return -1;
    }

    // Check if the kernel returned an internal subsystem error
    if (nlh->nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
        // err->error is typically a negative errno (e.g., -EINVAL)
        fprintf(stderr, "Kernel returned Netlink error code: %d\n", err->error);
        return -1;
    }

    return read_bytes;
}
