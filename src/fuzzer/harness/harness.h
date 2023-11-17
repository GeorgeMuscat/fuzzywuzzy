#pragma once

#include "socket.h"
#include <ucontext.h>

#define BUF_SIZE 8192
#define NUM_MMAPS 32
#define NUM_SIGNALS 32
#define NUM_REGIONS 16

#define MMAP_BASE 0x20000000

#define CTRL_OFFSET "0x7c"  // if everything stop working, check this

#define save_ra() \
void *ra = NULL;\
__asm__(\
        "mov %[asm_ra], [ebp+4]\n"\
        : [asm_ra] "=&r" (ra)\
)

#define save_ebx() \
void *ebx = NULL;\
__asm__(\
        "mov %[asm_ra], [ebx]\n"\
        : [asm_ra] "=&r" (ra)\
)

struct mmap_data {
    void *addr;
    size_t len;
};

struct memory_region {
    void *base;
    void *top;
    size_t size;

    void *saved_data;
};

struct control_data {
    void *eax;
    void *ecx;
    void *edx;
    void *ebx;
    void *esp;
    void *ebp;
    void *esi;
    void *edi;

    struct memory_region writable[NUM_REGIONS];
    size_t writable_index;
    void *writable_saved_base;
    void *writable_saved_curr;

    void* signals[NUM_SIGNALS];

    size_t mmap_index;
    struct mmap_data mmaps[NUM_MMAPS];

    int (*original_main_fn)(int, char **, char **);

    char buf[BUF_SIZE];
    volatile int *dummy_malloc;
    struct fuzzer_socket_t sock;

    ucontext_t context;
    int last_exit_code;
};

int fuzzywuzzy_main(int argc, char **argv, char **environ);
void fuzzywuzzy_read_mmap();

void fuzzywuzzy_log_start();
void fuzzywuzzy_log_reset(int exit_code);
void fuzzywuzzy_log_libc_call(const char *func_name, void *return_addr);

void fuzzywuzzy_reset(int exit_code);