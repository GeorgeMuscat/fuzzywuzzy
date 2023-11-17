#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <malloc.h>

#include "harness.h"


const char *heap_str = "[heap]";
const char *harness_str = "harness.so";

//do not use realloc or free ANYWHERE

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
        //endregion
        // we need to do a malloc to initialise the heap, and this needs to be the last item on the heap
        fuzzywuzzy_ctrl.dummy_malloc = malloc(0x8); //lolxd
        fuzzywuzzy_read_mmap();
    }


    //eax is about to be nuked, who cares what we do to it
    //save reg
    __asm__ (
            "lea eax, dword [ebx+"CTRL_OFFSET"]\n"
            "mov [eax + 0x00], eax\n"
            "mov [eax + 0x04], ecx\n"
            "mov [eax + 0x08], edx\n"
            "mov [eax + 0x0c], ebx\n"
            "mov [eax + 0x10], esp\n"
            "mov [eax + 0x14], ebp\n"
            "mov [eax + 0x18], esi\n"
            "mov [eax + 0x1c], edi\n"
            );

    // save writable memory regions
    __asm__(
            "mov edi, 0\n"                                                  // i = 0 [ctrl.writable]
            "fuzzywuzzy_save_loop_regions:\n"                               // for (; ; ) {
            "lea eax, dword [ebx+"CTRL_OFFSET"]\n"                          // eax = &fuzzywuzzy_ctrl
            "mov esi, [eax + 0x20 + 0x100]\n"                               // esi = writable_size
            "cmp edi, esi\n"                                                //
            "jge fuzzywuzzy_save_loop_regions_end\n"                        // if (i >= writable_size) break;
            "mov esi, edi\n"
            "shl edi, 4\n"
            "xchg esi, edi\n"
            "lea eax, [eax + 0x20 + esi]\n"                                 // eax = &ctrl.writable[i]
            "mov esi, 0\n"                                                  // j = 0 [ctrl.writable[i].saved_data]
            "fuzzywuzzy_save_loop_regions_loop_data:\n"                     // for(; ; ) {
            "mov edx, [eax + 0x0c]\n"                                       // edx = &saved[0]
            "lea edx, [edx + esi]\n"                                        // edx = &saved[i]
            "mov ecx, [eax + 0x00]\n"                                       // ecx = &base[0]
            "mov ecx, [ecx + esi]\n"                                        // ecx = base[i]
            "mov [edx], ecx\n"                                              // saved[i] = base[i]
            "mov edx, [eax + 0x08]\n"                                       // edx = size
            "add esi, 4\n"                                                  // i += 1
            "cmp esi, edx\n"                                                // cmp i + 1, stack_size
            "jne fuzzywuzzy_save_loop_regions_loop_data\n"                  // }
            "add edi, 1\n"                                                  // i++;
            "jmp fuzzywuzzy_save_loop_regions\n"                            // }
            "fuzzywuzzy_save_loop_regions_end:\n"
            );

    // restore registers
    __asm__(
            "lea eax, dword [ebx+"CTRL_OFFSET"]\n"
            "mov eax, [eax + 0x00]\n"
            "mov ecx, [eax + 0x04]\n"
            "mov edx, [eax + 0x08]\n"
            "mov ebx, [eax + 0x0c]\n"
            "mov esp, [eax + 0x10]\n"
            "mov ebp, [eax + 0x14]\n"
            "mov esi, [eax + 0x18]\n"
            "mov edi, [eax + 0x1c]\n"
            );


    __asm__("fuzzywuzzy_saved:\n");
    // this code will be run on every execution of the program
    //region C
    //endregion

    fuzzywuzzy_ctrl.original_main_fn(argc, argv, environ);
    fuzzywuzzy_reset();
}

/**
 * Reset program state, equivalent to re-running program. This will automatically re-invoke main
 * Can be called as a regular C function from anywhere
 */
_Noreturn void fuzzywuzzy_reset() {
    for (int i = 0; i < NUM_SIGNALS; i++) {
        if (fuzzywuzzy_ctrl.signals[i]) {
            fuzzywuzzy_ctrl.signals[i] = false;
            signal(i, SIG_DFL);
        }
    }

    for (int i = 0; i < fuzzywuzzy_ctrl.mmap_index; i++) {
        if (fuzzywuzzy_ctrl.mmaps[i].addr != NULL) {
            munmap(fuzzywuzzy_ctrl.mmaps[i].addr, fuzzywuzzy_ctrl.mmaps[i].len);
        }
    }


    fuzzywuzzy_ctrl.mmap_index = 0;
    // nuke writable memory regions
    __asm__(
            "mov edi, 0\n"                                                  // i = 0 [ctrl.writable]
            "fuzzywuzzy_nuke_loop_regions:\n"                               // for (; ; ) {
            "lea eax, dword [ebx+"CTRL_OFFSET"]\n"                          // eax = &fuzzywuzzy_ctrl
            "mov esi, [eax + 0x20 + 0x100]\n"                               // esi = writable_size
            "cmp edi, esi\n"                                                //
            "jge fuzzywuzzy_nuke_loop_regions_end\n"                        // if (i >= writable_size) break;
            "mov esi, edi\n"
            "shl edi, 4\n"
            "xchg esi, edi\n"
            "lea eax, [eax + 0x20 + esi]\n"                                 // eax = &ctrl.writable[i]
            "mov esi, 0\n"                                                  // j = 0 [ctrl.writable[i].saved_data]
            "fuzzywuzzy_nuke_loop_regions_loop_data:\n"                     // for(; ; ) {
            "mov edx, [eax + 0x0c]\n"                                       // edx = &saved[0]
            "mov edx, [edx + esi]\n"                                        // edx = saved[i]
            "mov ecx, [eax + 0x00]\n"                                       // ecx = &base[0]
            "lea ecx, [ecx + esi]\n"                                        // ecx = &base[i]
            "mov [ecx], edx\n"                                              // base[i] = saved[i]
            "mov edx, [eax + 0x08]\n"                                       // edx = size
            "add esi, 4\n"                                                  // i += 1
            "cmp esi, edx\n"                                                // cmp i + 1, stack_size
            "jne fuzzywuzzy_nuke_loop_regions_loop_data\n"                  // }
            "add edi, 1\n"                                                  // i++;
            "jmp fuzzywuzzy_nuke_loop_regions\n"                            // }
            "fuzzywuzzy_nuke_loop_regions_end:\n"
            );

    __asm__(
            "lea eax, dword [ebx+"CTRL_OFFSET"]\n"
            "mov eax, [eax + 0x00]\n"
            "mov ecx, [eax + 0x04]\n"
            "mov edx, [eax + 0x08]\n"
            "mov ebx, [eax + 0x0c]\n"
            "mov esp, [eax + 0x10]\n"
            "mov ebp, [eax + 0x14]\n"
            "mov esi, [eax + 0x18]\n"
            "mov edi, [eax + 0x1c]\n"
            );

    __asm__("jmp fuzzywuzzy_saved\n");

    // this never runs, but silences the no return warning
    exit(1);
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

/**
 * Read memory regions from mmap
 * Ensure malloc has been called at least once before calling this function
 */
void fuzzywuzzy_read_mmap() {
    int fd = open("/proc/self/maps", O_RDONLY);

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

    read(fd, buf, BUF_SIZE); //TODO: BUFFERED READ


    parse_state_t state = PARSE_STATE_ADDRS;
    int marker = 0;
    for (int i = 0; buf[i] != 0; i++) {
        switch (state) {
            case PARSE_STATE_ADDRS:
                if (buf[i] == '-') {
                    buf[i] = 0;
                    base = (void *) (strtoul(&buf[marker], NULL, 16));
                    buf[i] = '-';
                    marker = i + 1;
                } else if (buf[i] == ' ') {
                    buf[i] = 0;
                    top = (void *) (strtoul(&buf[marker], NULL, 16));
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
                    offset = strtoul(&buf[marker], NULL, 16);
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
                if (strncmp(&buf[name_start], heap_str, 6) == 0) {
                    heap_save_index = fuzzywuzzy_ctrl.writable_index;
                    top = (void*)fuzzywuzzy_ctrl.dummy_malloc + malloc_usable_size((void*)fuzzywuzzy_ctrl.dummy_malloc) + 0x8;
                    // size is slightly bigger, just to ensure we capture the next pointer
                }
                fuzzywuzzy_ctrl.writable[fuzzywuzzy_ctrl.writable_index] =
                        (struct memory_region) {base, top,top - base, NULL};

                total_size += fuzzywuzzy_ctrl.writable[fuzzywuzzy_ctrl.writable_index].top - fuzzywuzzy_ctrl.writable[fuzzywuzzy_ctrl.writable_index].base;
                fuzzywuzzy_ctrl.writable_index++;
            }

            state = PARSE_STATE_ADDRS;
            name_start = 0;
            marker = i + 1;
        }
    }

    fuzzywuzzy_ctrl.writable_saved_base = fuzzywuzzy_mmap((void*)MMAP_BASE, total_size, PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    fuzzywuzzy_ctrl.writable_saved_curr = fuzzywuzzy_ctrl.writable_saved_base;
    for (int i = 0; i < fuzzywuzzy_ctrl.writable_index; i++) {
        fuzzywuzzy_ctrl.writable[i].saved_data = fuzzywuzzy_ctrl.writable_saved_curr;
        fuzzywuzzy_ctrl.writable_saved_curr += fuzzywuzzy_ctrl.writable[i].size;
    }

    close(fd);
}


