#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <malloc.h>
#include <ucontext.h>

#include "harness.h"
#include "socket.h"
#include "hooks.h"

const char *heap_str = "[heap]";
const char *harness_str = "harness.so";

//do not use realloc or free ANYWHERE
__attribute__ ((noinline)) void fuzzywuzzy_restore();

void fuzzywuzzy_pre_reset(int exit_code);

void fuzzywuzzy_post_reset(int exit_code);

extern GEN_DEF(void_ptr mmap, void_ptr addr, size_t length, int prot, int flags, int fd, off_t offset)

extern GEN_DEF(int open, const_char_ptr pathname, int flags)

extern GEN_DEF(ssize_t read, int fd, void *buf, size_t count)

extern GEN_DEF(int close, int fd)

extern GEN_DEF(unsigned_long strtoul, const_char_ptr nptr, char_ptr_ptr endptr, int base)

extern GEN_DEF(int puts, const_char_ptr s)

extern GEN_DEF(size_t malloc_usable_size, void_ptr ptr)

extern GEN_DEF(int strncmp, const_char_ptr s1, const_char_ptr s2, size_t n)

extern GEN_DEF(char_ptr strcpy, char_ptr dest, const_char_ptr src)

extern GEN_DEF(void_ptr malloc, size_t size)

extern GEN_DEF(int munmap, void_ptr addr, size_t length)

extern void (*(*fuzzywuzzy_real_signal)(int, void (*func)(int)))(int);

struct control_data fuzzywuzzy_ctrl = {0};
struct control_data *fuzzywuzzy_ctrl_ptr = &fuzzywuzzy_ctrl;

/**
 * Injected main function, this should only be passed to __libc_start_main, never called directly
 * @param argc target argc
 * @param argv target argv
 * @param environ target environ
 * @return *does not return*
 */
int fuzzywuzzy_main(int argc, char **argv, char **environ) {
    if (!fuzzywuzzy_ctrl.dummy_malloc) {
        // you are also free to use malloc here, but atm everything that you malloc will be reset, that can be fixed if necessary
        //region C
        fuzzywuzzy_preload_hooks();
        setvbuf(stdin, fuzzywuzzy_ctrl.stdin_buf, _IOLBF, 65535);
        fuzzywuzzy_restore();
#ifdef __x86_64__
        fuzzywuzzy_ctrl.context.uc_mcontext.gregs[11] = (greg_t) &fuzzywuzzy_ctrl;
#else
        fuzzywuzzy_ctrl.context.uc_mcontext.gregs[8] = (greg_t) &fuzzywuzzy_ctrl;
#endif
        fuzzywuzzy_init_socket(&fuzzywuzzy_ctrl.sock);
        //endregion
        // we need to do a malloc to initialise the heap, and this needs to be the last item on the heap
        fuzzywuzzy_ctrl.dummy_malloc = REAL(malloc)(SMALL_MALLOC_SIZE); //lolxd
        fuzzywuzzy_read_mmap();
    }

#ifdef __x86_64__
    asm (
            "mov rax, %[ctrl]\n"
            "mov [rax], rax\n"
            "mov [rax + 0x08], rcx\n"
            "mov [rax + 0x10], rdx\n"
            "mov [rax + 0x18], rbx\n"
            "mov [rax + 0x20], rsp\n"
            "mov [rax + 0x28], rbp\n"
            "mov [rax + 0x30], rsi\n"
            "mov [rax + 0x38], rdi\n"
            "mov [rax + 0x40], r8\n"
            "mov [rax + 0x48], r9\n"
            "mov [rax + 0x50], r10\n"
            "mov [rax + 0x58], r11\n"
            "mov [rax + 0x60], r12\n"
            "mov [rax + 0x68], r13\n"
            "mov [rax + 0x70], r14\n"
            "mov [rax + 0x78], r15\n"
            :
            : [ctrl] "r"(fuzzywuzzy_ctrl_ptr)
            : "rax", "memory"
    );

    // save writable memory regions
    // save writable memory regions
    __asm__(
            "mov rdx, 0\n"                                                  // i = 0 [ctrl.writable]
            "cld\n"
            "fuzzywuzzy_save_loop_regions:\n"                               // for (; ; ) {
            "mov rax, %[ctrl]\n"
            "mov rcx, qword [rax+ "REG_SIZE" + "MEMORY_REGION_SIZE"]\n"      // ecx = writable_index
            "cmp rdx, rcx\n"                                                //
            "jge fuzzywuzzy_save_loop_regions_end\n"                        // if (i >= writable_size) break;
            "mov rcx, rdx\n"
            "shl rcx, 5\n"
            "lea rax, qword [rax + "REG_SIZE" + rcx]\n"                             // eax = &ctrl.writable[i]
            "mov rcx, qword [rax + 0x10]\n"
            "mov rsi, qword [rax + 0x00]\n"
            "mov rdi, qword [rax + 0x18]\n"
            "REP MOVS QWORD PTR ES:[RDI], QWORD PTR [RSI]\n"
            "add rdx, 1\n"                                                  // i++;
            "jmp fuzzywuzzy_save_loop_regions\n"                            // }
            "fuzzywuzzy_save_loop_regions_end:\n"
            :
            : [ctrl] "r"(fuzzywuzzy_ctrl_ptr)

    );

    // restore registers
    __asm__(
            "mov rax, %[ctrl]\n"
            "mov rcx, [rax + 0x08]\n"
            "mov rdx, [rax + 0x10]\n"
            "mov rbx, [rax + 0x18]\n"
            "mov rsp, [rax + 0x20]\n"
            "mov rbp, [rax + 0x28]\n"
            "mov rsi, [rax + 0x30]\n"
            "mov rdi, [rax + 0x38]\n"
            "mov r8,  [rax + 0x40]\n"
            "mov r9,  [rax + 0x48]\n"
            "mov r10, [rax + 0x50]\n"
            "mov r11, [rax + 0x58]\n"
            "mov r12, [rax + 0x60]\n"
            "mov r13, [rax + 0x68]\n"
            "mov r14, [rax + 0x70]\n"
            "mov r15, [rax + 0x78]\n"
            "mov rax, [rax + 0x00]\n"
            :
    : [ctrl] "r"(fuzzywuzzy_ctrl_ptr)
    : "rax", "rcx", "rdx", "rsi", "rdi", "memory"
    );
#else
    //eax is about to be nuked, who cares what we do to it
    //save reg
    __asm__ (
            "mov eax, %[ctrl]\n"
            "mov [eax + 0x00], eax\n"
            "mov [eax + 0x04], ecx\n"
            "mov [eax + 0x08], edx\n"
            "mov [eax + 0x0c], ebx\n"
            "mov [eax + 0x10], esp\n"
            "mov [eax + 0x14], ebp\n"
            "mov [eax + 0x18], esi\n"
            "mov [eax + 0x1c], edi\n"
            :
            : [ctrl] "r"(fuzzywuzzy_ctrl_ptr)
    : "memory"
    );

    // save writable memory regions
    __asm__(
            "mov edx, 0\n"                                                  // i = 0 [ctrl.writable]
            "cld\n"
            "fuzzywuzzy_save_loop_regions:\n"                               // for (; ; ) {
            "mov eax, %[ctrl]\n"
            "mov ecx, [eax + "REG_SIZE" + "MEMORY_REGION_SIZE"]\n"      // ecx = writable_index
            "cmp edx, ecx\n"                                                //
            "jge fuzzywuzzy_save_loop_regions_end\n"                        // if (i >= writable_size) break;
            "mov ecx, edx\n"
            "shl ecx, 4\n"
            "lea eax, [eax + "REG_SIZE" + ecx]\n"                             // eax = &ctrl.writable[i]
            "mov ecx, [eax + 0x08]\n"
            "mov esi, [eax + 0x00]\n"
            "mov edi, [eax + 0x0c]\n"
            "REP MOVS DWORD PTR ES:[EDI], DWORD PTR [ESI]\n"
            "add edx, 1\n"                                                  // i++;
            "jmp fuzzywuzzy_save_loop_regions\n"                            // }
            "fuzzywuzzy_save_loop_regions_end:\n"
            :
            : [ctrl] "r"(fuzzywuzzy_ctrl_ptr)
    : "eax", "ecx", "edx", "esi", "edi", "memory"
    );

    // restore registers
    __asm__(
            "mov eax, %[ctrl]\n"
            "mov ecx, [eax + 0x04]\n"
            "mov edx, [eax + 0x08]\n"
            "mov ebx, [eax + 0x0c]\n"
            "mov esp, [eax + 0x10]\n"
            "mov ebp, [eax + 0x14]\n"
            "mov esi, [eax + 0x18]\n"
            "mov edi, [eax + 0x1c]\n"
            "mov eax, [eax + 0x00]\n"
            :
            : [ctrl] "r"(fuzzywuzzy_ctrl_ptr)
            : "eax"
    );
#endif


    __asm__("jmp fuzzywuzzy_first_run\n");

    __asm__("fuzzywuzzy_reset_point:\n");
    fuzzywuzzy_post_reset(fuzzywuzzy_ctrl.last_exit_code);
    __asm__("fuzzywuzzy_first_run:\n");

    // this code will be run on every execution of the program
    fuzzywuzzy_log_start();
    int exit_code = fuzzywuzzy_ctrl.original_main_fn(argc, argv, environ);
    fuzzywuzzy_reset(exit_code);
}

/**
 * Logs a call to libc to the current socket connection.
 */
void fuzzywuzzy_log_libc_call(const char *func_name, void *return_addr) {
    struct fuzzer_msg_t msg = {.msg_type = MSG_LIBC_CALL, .data = {.libc_call = {"", return_addr}}};
    REAL(strcpy)(msg.data.libc_call.func_name, func_name);
    fuzzywuzzy_write_message(&fuzzywuzzy_ctrl.sock, &msg);
    //fuzzywuzzy_expect_ack(&fuzzywuzzy_ctrl.sock);
}

/**
 * Logs a start to the current socket connection.
 */
void fuzzywuzzy_log_start() {
    struct fuzzer_msg_t msg = {.msg_type = MSG_TARGET_START, .data = {}};
    fuzzywuzzy_write_message(&fuzzywuzzy_ctrl.sock, &msg);
    fuzzywuzzy_expect_ack(&fuzzywuzzy_ctrl.sock);
}

/**
 * Logs a reset to the current socket connection.
 */
void fuzzywuzzy_log_reset(int exit_code) {
    struct fuzzer_msg_t msg = {.msg_type = MSG_TARGET_RESET, .data = {.target_reset = {exit_code}}};
    fuzzywuzzy_write_message(&fuzzywuzzy_ctrl.sock, &msg);
    fuzzywuzzy_expect_ack(&fuzzywuzzy_ctrl.sock);
}

void fuzzywuzzy_reset(int exit_code) {
    fuzzywuzzy_pre_reset(exit_code);
    setcontext(&fuzzywuzzy_ctrl.context);
}

void fuzzywuzzy_pre_reset(int exit_code) {
    fuzzywuzzy_ctrl.last_exit_code = exit_code;
    // Resets signal handlers.
    for (int i = 0; i < NUM_SIGNALS; i++) {
        if (fuzzywuzzy_ctrl.signals[i]) {
            fuzzywuzzy_ctrl.signals[i] = false;
            REAL(signal)(i, SIG_DFL);
        }
    }

    // Unmaps memory regions.
    for (int i = 0; i < fuzzywuzzy_ctrl.mmap_index; i++) {
        if (fuzzywuzzy_ctrl.mmaps[i].addr != NULL) {
            REAL(munmap)(fuzzywuzzy_ctrl.mmaps[i].addr, fuzzywuzzy_ctrl.mmaps[i].len);
        }
    }

    fuzzywuzzy_ctrl.mmap_index = 0;
}

/**
 * Performs operations to reset program state other than memory restoration.
 */
void fuzzywuzzy_post_reset(int exit_code) {
    //close(STDIN_FILENO);
    // Flush stdin stream.
    //fseek(stdin,0,SEEK_END);
    ungetc(0, stdin);
    __fpurge(stdin);

    // Logs reset event to socket connection.
    fuzzywuzzy_log_reset(exit_code);
}

/**
 * Restore program state to be equivalent to program start. Should be used via setcontext and only called as a function for setup.
 */
__attribute__ ((noinline)) void fuzzywuzzy_restore() {
    if (fuzzywuzzy_ctrl.dummy_malloc == NULL) {
        getcontext(&fuzzywuzzy_ctrl.context);
#ifdef __x86_64__
        //fuzzywuzzy_ctrl.context.uc_mcontext.gregs[16] += 1;
        fuzzywuzzy_ctrl.context.uc_mcontext.gregs[16] += 0x29;

#else
        //fuzzywuzzy_ctrl.context.uc_mcontext.gregs[2] += 23;
        fuzzywuzzy_ctrl.context.uc_mcontext.gregs[14] += 0x23;
#endif
        return;
    }

    // NO CODE CAN GO HERE. NOTHING. ZERO.

    // nuke writable memory regions
#ifdef __x86_64__
    __asm__ (
            "mov rdx, 0\n"                                                  // i = 0 [ctrl.writable]
            "cld\n"
            "fuzzywuzzy_nuke_loop_regions:\n"                               // for (;;) {
            "mov rax, rbx\n"
            "mov rcx, [rax + "REG_SIZE" + "MEMORY_REGION_SIZE"]\n"          // ecx = writable_index
            "cmp rdx, rcx\n"                                                //
            "jge fuzzywuzzy_nuke_loop_regions_end\n"                        // if (i >= writable_size) break;
            "mov rcx, rdx\n"
            "shl rcx, 5\n"
            "lea rax, [rax + "REG_SIZE" + rcx]\n"                             // eax = &ctrl.writable[i]
            "mov rcx, [rax + 0x10]\n"
            "mov rdi, qword [rax + 0x00]\n"
            "mov rsi, qword [rax + 0x18]\n"
            "REP MOVS QWORD PTR ES:[RDI], QWORD PTR [RSI]\n"
            "add rdx, 1\n"                                                  // i++;
            "jmp fuzzywuzzy_nuke_loop_regions\n"                            // }
            "fuzzywuzzy_nuke_loop_regions_end:\n");

    __asm__(
            "mov rax, rbx\n"
            "mov rcx, [rax + 0x08]\n"
            "mov rdx, [rax + 0x10]\n"
            "mov rbx, [rax + 0x18]\n"
            "mov rsp, [rax + 0x20]\n"
            "mov rbp, [rax + 0x28]\n"
            "mov rsi, [rax + 0x30]\n"
            "mov rdi, [rax + 0x38]\n"
            "mov r8,  [rax + 0x40]\n"
            "mov r9,  [rax + 0x48]\n"
            "mov r10, [rax + 0x50]\n"
            "mov r11, [rax + 0x58]\n"
            "mov r12, [rax + 0x60]\n"
            "mov r13, [rax + 0x68]\n"
            "mov r14, [rax + 0x70]\n"
            "mov r15, [rax + 0x78]\n"
            "mov rax, [rax + 0x00]\n"
            );

    __asm__("jmp fuzzywuzzy_reset_point\n");
#else
    __asm__ (
            "mov edx, 0\n"                                                  // i = 0 [ctrl.writable]
            "cld\n"
            "fuzzywuzzy_nuke_loop_regions:\n"                               // for (;;) {
            "mov ecx, [ebx + "REG_SIZE" + "MEMORY_REGION_SIZE"]\n"      // ecx = writable_index
            "cmp edx, ecx\n"                                                //
            "jge fuzzywuzzy_nuke_loop_regions_end\n"                        // if (i >= writable_size) break;
            "mov ecx, edx\n"
            "shl ecx, 4\n"
            "lea eax, [ebx + "REG_SIZE" + ecx]\n"                             // eax = &ctrl.writable[i]
            "mov ecx, [eax + 0x08]\n"
            "mov edi, [eax + 0x00]\n"
            "mov esi, [eax + 0x0c]\n"
            "REP MOVS DWORD PTR ES:[EDI], DWORD PTR [ESI]\n"
            "add edx, 1\n"                                                  // i++;
            "jmp fuzzywuzzy_nuke_loop_regions\n"                            // }
            "fuzzywuzzy_nuke_loop_regions_end:\n");

    __asm__  (
            "mov eax, ebx\n"
            "mov ecx, [eax + 0x04]\n"
            "mov edx, [eax + 0x08]\n"
            "mov ebx, [eax + 0x0c]\n"
            "mov esp, [eax + 0x10]\n"
            "mov ebp, [eax + 0x14]\n"
            "mov esi, [eax + 0x18]\n"
            "mov edi, [eax + 0x1c]\n"
            "mov eax, [eax + 0x00]\n");

    __asm__("jmp fuzzywuzzy_reset_point\n");
#endif
    // this never runs, but silences the no return warning
    for (;;);
}

typedef enum parse_state {
    PARSE_STATE_ADDRS,
    PARSE_STATE_PROT,
    PARSE_STATE_NAME,
    PARSE_STATE_OFFSET,
    PARSE_STATE_DEVICE,
    PARSE_STATE_INODE,
    PARSE_STATE_DONE,
} parse_state_t;


#ifdef __x86_64__
void *round_up(void *x) {
    return (void *) ((size_t) x & 0xFFFFFF70) + 0x20;
}
#else
void *round_up(void *x) {
    return (void *) ((size_t) x & 0xFFFFFFF0) + 0x10;
}
#endif

/**
 * Read memory regions from mmap
 * Ensure malloc has been called at least once before calling this function
 */
void fuzzywuzzy_read_mmap() {
    int fd = REAL(open)("/proc/self/maps", O_RDONLY);

    void *base = NULL;
    void *top = NULL;
    char prot[5] = {0};
    size_t offset = 0;
    int name_start = 0;
    int name_end = 0;
    size_t heap_save_index = 0;
    void *last_heap_addr = NULL;
    size_t last_heap_size = 0;
    size_t total_size = 0;


    char *buf = fuzzywuzzy_ctrl.buf;

    REAL(read)(fd, buf, BUF_SIZE); //TODO: BUFFERED READ


    parse_state_t state = PARSE_STATE_ADDRS;
    int marker = 0;
    for (int i = 0; buf[i] != 0; i++) {
        switch (state) {
            case PARSE_STATE_ADDRS:
                if (buf[i] == '-') {
                    buf[i] = 0;
                    base = (void *) (REAL(strtoul)(&buf[marker], NULL, 16));
                    buf[i] = '-';
                    marker = i + 1;
                } else if (buf[i] == ' ') {
                    buf[i] = 0;
                    top = (void *) (REAL(strtoul)(&buf[marker], NULL, 16));
                    buf[i] = ' ';
                    marker = i + 1;
                    state = PARSE_STATE_PROT;
                }
                break;
            case PARSE_STATE_PROT:
                if (buf[i] == ' ') {
                    state = PARSE_STATE_OFFSET;
                    marker = i + 1;
                } else {
                    prot[i - marker] = buf[i];
                }
                break;
            case PARSE_STATE_OFFSET:
                if (buf[i] == ' ') {
                    buf[i] = 0;
                    offset = REAL(strtoul)(&buf[marker], NULL, 16);
                    buf[i] = ' ';
                    state = PARSE_STATE_DEVICE;
                    marker = i + 1;
                }
                break;
            case PARSE_STATE_DEVICE:
                if (buf[i] == ' ') {
                    marker = i + 1;
                    state = PARSE_STATE_INODE;
                }
                break;
            case PARSE_STATE_INODE:
                if (buf[i] == ' ') {
                    marker = i + 1;
                    state = PARSE_STATE_NAME;
                }
                break;
            case PARSE_STATE_NAME:
                if (name_start == 0 && buf[i] != ' ') {
                    name_start = i;
                } else if (buf[i] == '\n') {
                    name_end = i - 1;
                    state = PARSE_STATE_DONE;
                }
                break;
            case PARSE_STATE_DONE:
                break;
        }

        if (state == PARSE_STATE_DONE) {
            bool should_save = true;
            if (name_end - name_start == 10) {
                for (int j = name_start; j <= name_end; j--) {
                    if (buf[name_start] != harness_str[j - name_start]) {
                        should_save = false;
                    }
                }
            }
            if (prot[1] != 'w') {
                should_save = false;
            }


            if (should_save) {
                if (REAL(strncmp)(&buf[name_start], heap_str, 6) == 0) {
                    heap_save_index = fuzzywuzzy_ctrl.writable_index;
                    top = (void *) fuzzywuzzy_ctrl.dummy_malloc +
                          REAL(malloc_usable_size)((void *) fuzzywuzzy_ctrl.dummy_malloc) + SMALL_MALLOC_SIZE;
                    top = round_up(top);
                    // size is slightly bigger, just to ensure we capture the next pointer
                }
#ifdef __x86_64__
                fuzzywuzzy_ctrl.writable[fuzzywuzzy_ctrl.writable_index] =
                        (struct memory_region) {base, top, (top - base) >> 3, NULL};
#else
                fuzzywuzzy_ctrl.writable[fuzzywuzzy_ctrl.writable_index] =
                        (struct memory_region) {base, top,(top - base) >> 2, NULL};
#endif


                total_size += fuzzywuzzy_ctrl.writable[fuzzywuzzy_ctrl.writable_index].top -
                              fuzzywuzzy_ctrl.writable[fuzzywuzzy_ctrl.writable_index].base;
                fuzzywuzzy_ctrl.writable_index++;
            }

            state = PARSE_STATE_ADDRS;
            name_start = 0;
            marker = i + 1;
        }
    }

    fuzzywuzzy_ctrl.writable_saved_base = REAL(mmap)((void *) MMAP_BASE, total_size, PROT_WRITE | PROT_READ,
                                                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    fuzzywuzzy_ctrl.writable_saved_curr = fuzzywuzzy_ctrl.writable_saved_base;
    for (int i = 0; i < fuzzywuzzy_ctrl.writable_index; i++) {
        fuzzywuzzy_ctrl.writable[i].saved_data = fuzzywuzzy_ctrl.writable_saved_curr;
#ifdef __x86_64__
        fuzzywuzzy_ctrl.writable_saved_curr += (fuzzywuzzy_ctrl.writable[i].size_in_qwords << 3);
#else
        fuzzywuzzy_ctrl.writable_saved_curr += (fuzzywuzzy_ctrl.writable[i].size_in_dwords << 2);
#endif
    }

    REAL(close)(fd);
}


