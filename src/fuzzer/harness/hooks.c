#define _GNU_SOURCE
#include "hooks.h"
#include <stdio.h>

#include <dlfcn.h>

#include "harness.h"

void (*real_free)(void *ptr);
void (*real_exit)(int status);
void *(*real_mmap)(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int (*real_munmap)(void *addr, size_t length);
void (*(*real_signal)(int, void (*func)(int)))(int);
int *(*real_libc_start_main)(int (*main)(int, char **, char **), int argc, char **ubp_av, void (*init)(void),
                             void (*fini)(void), void (*rtld_fini)(void), void(*stack_end));
int (*real_socket)(int domain, int type, int protocol);

void *fuzzywuzzy_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

extern struct control_data fuzzywuzzy_ctrl;

void fuzzywuzzy_preload_hooks(void) {
    *(void **)(&real_free) = dlsym(RTLD_NEXT, "free");
    *(void **)(&real_exit) = dlsym(RTLD_NEXT, "exit");
    *(void **)(&real_signal) = dlsym(RTLD_NEXT, "signal");
    *(void **)(&real_mmap) = dlsym(RTLD_NEXT, "mmap");
    *(void **)(&real_munmap) = dlsym(RTLD_NEXT, "munmap");
    *(void **)(&real_libc_start_main) = dlsym(RTLD_NEXT, "__libc_start_main");
    // *(void **)(&real_socket) = dlsym(RTLD_NEXT, "socket");
    // *(void **)(&real_abort) = dlsym(RTLD_NEXT, "abort");
}

void free(void *ptr) {
    if (!real_free) {
        *(void **)(&real_free) = dlsym(RTLD_NEXT, __func__);
    }
    (*real_free)(ptr);
}

_Noreturn void exit(int status) {
    if (!real_exit) {
        *(void **)(&real_exit) = dlsym(RTLD_NEXT, __func__);
    }
    fuzzywuzzy_reset(status);

    (*real_exit)(status);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    if (!real_mmap) {
        *(void **)(&real_mmap) = dlsym(RTLD_NEXT, __func__);
    }

    void *res = (*real_mmap)(addr, length, prot, flags, fd, offset);

    fuzzywuzzy_ctrl.mmaps[fuzzywuzzy_ctrl.mmap_index++] = (struct mmap_data){res, length};

    return res;
}

void *fuzzywuzzy_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    if (!real_mmap) {
        *(void **)(&real_mmap) = dlsym(RTLD_NEXT, "mmap");
    }

    return (*real_mmap)(addr, length, prot, flags, fd, offset);
}

int munmap(void *addr, size_t length) {
    if (!real_munmap) {
        *(void **)(&real_munmap) = dlsym(RTLD_NEXT, __func__);
    }

    int res = (*real_munmap)(addr, length);

    // todo: hashmap this shit (but its C, do we consider C++?)
    for (int i = 0; i < fuzzywuzzy_ctrl.mmap_index; i++) {
        if (fuzzywuzzy_ctrl.mmaps[i].addr == addr) {
            fuzzywuzzy_ctrl.mmaps[i].addr = NULL;
            if (i == fuzzywuzzy_ctrl.mmap_index - 1) {
                fuzzywuzzy_ctrl.mmap_index -= 1;
            }
            break;
        }
    }

    return res;
}

void (*signal(int sig, void (*func)(int)))(int) {
    if (!real_signal) {
        *(void **)(&real_signal) = dlsym(RTLD_NEXT, __func__);
    }
    fuzzywuzzy_ctrl.signals[sig] = true;
    printf("registering signal for %d\n", sig);

    return (*real_signal)(sig, func);
}

int *__libc_start_main(int (*main)(int, char **, char **), int argc, char **ubp_av, void (*init)(void), void (*fini)(void),
                       void (*rtld_fini)(void), void(*stack_end)) {
    if (!real_libc_start_main) {
        *(void **)(&real_libc_start_main) = dlsym(RTLD_NEXT, __func__);
    }

    if (fuzzywuzzy_ctrl.original_main_fn != NULL) {
        puts("WARNING: LIBC START MAIN TWICE, THIS WILL BREAK THE HARNESS");
        abort();
    }

    fuzzywuzzy_ctrl.original_main_fn = main;

    return (*real_libc_start_main)(fuzzywuzzy_main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}