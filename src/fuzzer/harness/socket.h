#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#define SOCKET_PATH_ENVVAR "FUZZYWUZZY_SOCKET_PATH"
#define MAX_FUNCTION_NAME_LENGTH 64

#define MSG_ACK 0x01
#define MSG_TARGET_START 0x02
#define MSG_TARGET_RESET 0x03
#define MSG_LIBC_CALL 0x04
#define MSG_INPUT_REQUIRED 0x05
#define MSG_INPUT_RESPONSE 0x06

struct fuzzer_socket_t {
    int conn_fd;
};

struct fuzzer_msg_t {
    uint8_t msg_type;
    union fuzzer_msg_data_t {
        // 0x01 - expected from the fuzzer to acknowledge a message that does not require a reply.
        struct fuzzer_msg_ack_t {
        } ack;
        // 0x02 - used to indicate that the target binary's `main` function is about to be started.
        struct fuzzer_msg_target_start_t {
        } target_start;
        // 0x03 - indicates that the target binary is about to be reset.
        struct fuzzer_msg_target_reset_t {
            // Intended exit code of the target binary.
            int exit_code;
        } target_reset;
        // 0x04 - logs a call to a libc function for coverage purposes.
        struct fuzzer_msg_libc_call_t {
            // Name of called function.
            char func_name[MAX_FUNCTION_NAME_LENGTH];
            // Location in the target binary that will be returned to.
            // Used for pseudo-coverage.
            void *return_addr;
        } libc_call;
        // 0x05 - requests a check from the fuzzer whether the current input will satisfy the current input request (e.g. fgets call)
        struct fuzzer_msg_input_required_t {
            /*
            0x1 - has length
            0x2 - terminates on \0
            0x4 - terminates on \n
            0x8 - adds \0 to end of input (reads `len-1` bytes)
            */
            uint8_t flags;
            // Maximum length of input, if applicable.
            size_t len;
        } input_required;
        // 0x06 - reply to `input_required`
        struct fuzzer_msg_input_response_t {
            // Is the fuzzer providing enough input to satisfy the current call?
            bool can_satisfy;
        } input_response;
    } data;
};

void fuzzywuzzy_init_socket(struct fuzzer_socket_t *sock);
int fuzzywuzzy_read_message(struct fuzzer_socket_t *sock, struct fuzzer_msg_t *msg);
int fuzzywuzzy_write_message(struct fuzzer_socket_t *sock, struct fuzzer_msg_t *msg);
void fuzzywuzzy_expect_ack(struct fuzzer_socket_t *sock);
void fuzzywuzzy_close_socket(struct fuzzer_socket_t *sock);