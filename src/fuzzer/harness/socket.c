#include "socket.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "hooks.h"

extern GEN_DEF(char_ptr getenv, const_char_ptr name) extern GEN_DEF(char_ptr strcpy, char_ptr dest, const_char_ptr src);
extern GEN_DEF(int socket, int domain, int type, int protocol);
extern GEN_DEF(void abort);
extern GEN_DEF(size_t strlen, const_char_ptr s);
extern GEN_DEF(int connect, int sockfd, const_sockaddr_ptr addr, socklen_t addrlen);
extern GEN_DEF(void_ptr memset, void_ptr s, int c, size_t n);
extern GEN_DEF(ssize_t read, int fd, void *buf, size_t count);
extern GEN_DEF(ssize_t write, int fd, const_void_ptr buf, size_t count);
extern GEN_DEF(int close, int fd);
extern GEN_DEF(int puts, const_char_ptr s);

void fuzzywuzzy_init_socket(struct fuzzer_socket_t *sock) {
    char *path = REAL(getenv, SOCKET_PATH_ENVVAR);

    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;
    REAL(strcpy, remote.sun_path, path);

    int sock_fd = REAL(socket, AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        REAL(abort);
    }

    int remote_len = sizeof(remote.sun_family) + REAL(strlen, remote.sun_path);
    int result = REAL(connect, sock_fd, (struct sockaddr *)&remote, remote_len);
    if (result < 0) {
        REAL(abort);
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
    REAL(memset, msg, 0, sizeof(struct fuzzer_msg_t));

    REAL(read, sock->conn_fd, &msg->msg_type, 1);
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
            REAL(read, sock->conn_fd, &msg->data.input_response.can_satisfy, 1);
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

    REAL(write, sock->conn_fd, &msg->msg_type, 1);
    if (data_size) {
        REAL(write, sock->conn_fd, &msg->data, data_size);
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
        REAL(abort);
    }
}

void fuzzywuzzy_close_socket(struct fuzzer_socket_t *sock) {
    REAL(close, sock->conn_fd);
}