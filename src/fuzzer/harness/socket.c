#include "socket.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// #include "harness.h"

void fuzzywuzzy_init_socket(struct fuzzer_socket_t *sock) {
    char *path = getenv(SOCKET_PATH_ENVVAR);

    struct sockaddr_un local;
    struct sockaddr_un remote;
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, path);

    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        abort();
    }

    int local_len = sizeof(local.sun_family) + strlen(local.sun_path);
    int bind_result = bind(sock_fd, (struct sockaddr *)&local, local_len);
    if (bind_result < 0) {
        abort();
    }

    // Begin listening for new connections.
    // Only allow 1 connection, refuse all others.
    listen(sock_fd, 0);

    int remote_len = sizeof(remote);
    int client_fd = accept(sock_fd, (struct sockaddr *)&remote, &remote_len);

    if (client_fd < 0) {
        abort();
    }

    sock->sock_fd = sock_fd;
    sock->client_fd = client_fd;
}

/**
 * Reads a message from the fuzzer.
 * @param sock socket info
 * @param msg message buffer
 * @return status (negative for error, 0 on success)
 */
int fuzzywuzzy_read_message(struct fuzzer_socket_t *sock, struct fuzzer_msg_t *msg) {
    memset(msg, 0, sizeof(struct fuzzer_msg_t));

    read(sock->client_fd, &msg->msg_type, 1);
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
            read(sock->client_fd, &msg->data.input_response.can_satisfy, 1);
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

    write(sock->client_fd, &msg->msg_type, 1);
    if (data_size) {
        write(sock->client_fd, &msg->data, data_size);
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
        abort();
    }
}

void fuzzywuzzy_close_socket(struct fuzzer_socket_t *sock) {
    close(sock->sock_fd);
    close(sock->client_fd);
}