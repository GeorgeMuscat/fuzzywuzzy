#include "socket.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

extern char *(*real_getenv)(const char *name);
extern char *(*real_strcpy)(char *restrict dest, const char *src);
extern int (*real_socket)(int domain, int type, int protocol);
extern void (*real_abort)(void);
extern size_t (*real_strlen)(const char *s);
extern int (*real_connect)(int sockfd, const struct sockaddr *addr,
                           socklen_t addrlen);
extern void *(*real_memset)(void *s, int c, size_t n);
extern ssize_t (*real_read)(int fd, void *buf, size_t count);
extern ssize_t (*real_write)(int fd, const void *buf, size_t count);
extern int (*real_close)(int fd);

void fuzzywuzzy_init_socket(struct fuzzer_socket_t *sock) {
    char *path = (*real_getenv)(SOCKET_PATH_ENVVAR);

    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;
    (*real_strcpy)(remote.sun_path, path);

    int sock_fd = (*real_socket)(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        (*real_abort)();
    }

    int remote_len = sizeof(remote.sun_family) + (*real_strlen)(remote.sun_path);
    int result = (*real_connect)(sock_fd, (struct sockaddr *)&remote, remote_len);
    if (result < 0) {
        (*real_abort)();
    }

    sock->conn_fd = sock_fd;
}

/**
 * Reads a message from the fuzzer.
 * @param sock socket info
 * @param msg message buffer
 * @return status (negative for error, 0 on success)
 */
int fuzzywuzzy_read_message(struct fuzzer_socket_t *sock, struct fuzzer_msg_t *msg) {
    (*real_memset)(msg, 0, sizeof(struct fuzzer_msg_t));

    (*real_read)(sock->conn_fd, &msg->msg_type, 1);
    switch (msg->msg_type) {
        case MSG_ACK:
            break;
        // Harness -> Fuzzer only.
        case MSG_TARGET_START:
        case MSG_TARGET_RESET:
        case MSG_LIBC_CALL:
        case MSG_INPUT_REQUIRED:
            // Unexpected message type.
            return -1;
        case MSG_INPUT_RESPONSE:
            (*real_read)(sock->conn_fd, &msg->data.input_response.can_satisfy, 1);
            break;
        default:
            // Unknown message type.
            return -2;
    }

    return 0;
}

/**
 * Writes a message to the fuzzer.
 * @param sock socket info
 * @param msg message
 * @return status (negative for error, 0 on success)
 */
int fuzzywuzzy_write_message(struct fuzzer_socket_t *sock, struct fuzzer_msg_t *msg) {
    int data_size = 0;

    switch (msg->msg_type) {
        case MSG_TARGET_START:
            data_size = sizeof(struct fuzzer_msg_target_start_t);
            break;
        case MSG_TARGET_RESET:
            data_size = sizeof(struct fuzzer_msg_target_reset_t);
            break;
        case MSG_LIBC_CALL:
            data_size = sizeof(struct fuzzer_msg_libc_call_t);
            break;
        case MSG_INPUT_REQUIRED:
            data_size = sizeof(struct fuzzer_msg_input_required_t);
            break;
        // Fuzzer -> Harness only.
        case MSG_ACK:
        case MSG_INPUT_RESPONSE:
            // Unexpected message type.
            return -1;
        default:
            // Unknown message type.
            return -2;
    }

    (*real_write)(sock->conn_fd, &msg->msg_type, 1);
    if (data_size) {
        (*real_write)(sock->conn_fd, &msg->data, data_size);
    }

    return 0;
}

/**
 * Waits for and reads a message from the current socket connection and aborts if it is not a MSG_ACK.
 */
void fuzzywuzzy_expect_ack(struct fuzzer_socket_t *sock) {
    struct fuzzer_msg_t msg = {0};
    fuzzywuzzy_read_message(sock, &msg);

    if (msg.msg_type != MSG_ACK) {
        (*real_abort)();
    }
}

void fuzzywuzzy_close_socket(struct fuzzer_socket_t *sock) {
    (*real_close)(sock->conn_fd);
}