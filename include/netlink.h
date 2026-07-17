#ifndef NETLINK_H
#define NETLINK_H

#include <sys/types.h>
#include <stdint.h>
#include <linux/rtnetlink.h>

#define BUFFER_SIZE 16384

/**
 * Sends a raw Netlink request message to the kernel.
 * 
 * @param sock_fd      The open raw Netlink socket descriptor.
 * @param msg_type     Netlink/RTNETLINK message type (e.g. RTM_GETLINK).
 * @param flags        Request control flags (e.g. NLM_F_REQUEST | NLM_F_DUMP).
 * @param seq_num      Sequence number for correlating requests with replies.
 * @param payload      Pointer to the protocol family message payload (e.g. struct rtgenmsg).
 * @param payload_len  Size of the payload structure in bytes.
 * @return             0 on success, -1 on failure.
 */
int netlink_send_helper(int sock_fd, uint16_t msg_type, uint16_t flags, 
                        uint32_t seq_num, const void *payload, size_t payload_len);

/**
 * Reads a single raw packet from the Netlink socket.
 * 
 * @param sock_fd      The open raw Netlink socket descriptor.
 * @param out_buffer   Destination buffer to store the received packet.
 * @param buffer_len   Maximum capacity of the destination buffer.
 * @return             The number of bytes read on success, -1 on failure.
 */
ssize_t netlink_receive_helper(int sock_fd, char *out_buffer, size_t buffer_len);

#endif // NETLINK_H
