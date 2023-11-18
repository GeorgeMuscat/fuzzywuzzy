#pragma once

#include <ucontext.h>

#include "socket.h"

#define BUF_SIZE 8192
#define NUM_MMAPS 32
#define NUM_SIGNALS 32
#define NUM_REGIONS 16

#define MMAP_BASE 0x20000000

#ifdef __x86_64__
#define SMALL_MALLOC_SIZE 0x10
#else
#define SMALL_MALLOC_SIZE 0x08
#endif

#ifdef __x86_64__
#define CTRL_OFFSET "0x100"  // if everything stop working, check this
#else
#define CTRL_OFFSET "0xa0"  // if everything stop working, check this
#endif

#ifdef __x86_64__
#define save_ra()                  \
void *ra = NULL;                   \
    __asm__(                       \
        "mov %[asm_ra], [rbp+8]\n" \
        : [asm_ra] "=&r"(ra))
#else
#define save_ra()                  \
    void *ra = NULL;               \
    __asm__(                       \
        "mov %[asm_ra], [ebp+4]\n" \
        : [asm_ra] "=&r"(ra))
#endif

#define save_ebx()               \
    void *ebx = NULL;            \
    __asm__(                     \
        "mov %[asm_ra], [ebx]\n" \
        : [asm_ra] "=&r"(ra))

struct mmap_data {
    void *addr;
    size_t len;
};

struct memory_region {
    void *base;
    void *top;
#ifdef __x86_64__
    size_t size_in_qwords;
#else
    size_t size_in_dwords;
#endif


    void *saved_data;
};

#ifdef __x86_64__
#define REG_SIZE "0x80"
#define MEMORY_REGION_SIZE "0x200"
#else
#define REG_SIZE "0x20"
#define MEMORY_REGION_SIZE "0x100"
#endif

struct control_data {
#ifdef __x86_64
    void *rax;
    void *rcx;
    void *rdx;
    void *rbx;
    void *rsp;
    void *rbp;
    void *rsi;
    void *rdi;
    void *r8;
    void *r9;
    void *r10;
    void *r11;
    void *r12;
    void *r13;
    void *r14;
    void *r15;
#else
    void *eax;
    void *ecx;
    void *edx;
    void *ebx;
    void *esp;
    void *ebp;
    void *esi;
    void *edi;
#endif

    struct memory_region writable[NUM_REGIONS];
    size_t writable_index;
    void *writable_saved_base;
    void *writable_saved_curr;

    void *signals[NUM_SIGNALS];

    size_t mmap_index;
    struct mmap_data mmaps[NUM_MMAPS];

    int (*original_main_fn)(int, char **, char **);

    char buf[BUF_SIZE];
    volatile int *dummy_malloc;
    struct fuzzer_socket_t sock;

    ucontext_t context;
    int last_exit_code;
    int stdin_cpy;
    char stdin_buf[65535];
};

int fuzzywuzzy_main(int argc, char **argv, char **environ);
void fuzzywuzzy_read_mmap();

void fuzzywuzzy_log_start();
void fuzzywuzzy_log_reset(int exit_code);
void fuzzywuzzy_log_libc_call(const char *func_name, void *return_addr);


void fuzzywuzzy_reset(int reset_code);
