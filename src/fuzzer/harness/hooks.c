#define _GNU_SOURCE
#include "hooks.h"

#include <dlfcn.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdarg.h>

#include "harness.h"

void (*real_free)(void *ptr);
void (*real_exit)(int status);
void *(*real_mmap)(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int (*real_munmap)(void *addr, size_t length);
void (*(*real_signal)(int, void (*func)(int)))(int);
int *(*real___libc_start_main)(int (*main)(int, char **, char **), int argc, char **ubp_av, void (*init)(void),
                               void (*fini)(void), void (*rtld_fini)(void), void(*stack_end));
int (*real_puts)(const char *s);
char *(*real_getenv)(const char *name);
char *(*real_strcpy)(char *restrict dest, const char *src);
int (*real_socket)(int domain, int type, int protocol);
void (*real_abort)(void);
size_t (*real_strlen)(const char *s);
int (*real_bind)(int sockfd, const struct sockaddr *addr,
                 socklen_t addrlen);
int (*real_listen)(int sockfd, int backlog);
int (*real_accept)(int sockfd, struct sockaddr *restrict addr,
                   socklen_t *restrict addrlen);
void *(*real_memset)(void *s, int c, size_t n);
ssize_t (*real_read)(int fd, void *buf, size_t count);
ssize_t (*real_write)(int fd, const void *buf, size_t count);
int (*real_close)(int fd);
int (*real_vprintf)(const char *restrict format, va_list ap);

extern struct control_data fuzzywuzzy_ctrl;

void fuzzywuzzy_preload_hooks(void) {
    LOAD(free);
    LOAD(exit);
    LOAD(signal);
    LOAD(mmap);
    LOAD(munmap);
    LOAD(__libc_start_main);
    LOAD(puts);
    LOAD(getenv);
    LOAD(strcpy);
    LOAD(socket);
    LOAD(abort);
    LOAD(strlen);
    LOAD(bind);
    LOAD(listen);
    LOAD(accept);
    LOAD(memset);
    LOAD(read);
    LOAD(write);
    LOAD(close);
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

    // this'll never return :)
    for (;;)
        ;
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

    return (*real_signal)(sig, func);
}

int *__libc_start_main(int (*main)(int, char **, char **), int argc, char **ubp_av, void (*init)(void), void (*fini)(void),
                       void (*rtld_fini)(void), void(*stack_end)) {
    if (!real___libc_start_main) {
        *(void **)(&real___libc_start_main) = dlsym(RTLD_NEXT, __func__);
    }

    if (fuzzywuzzy_ctrl.original_main_fn != NULL) {
        (*real_puts)("WARNING: LIBC START MAIN TWICE, THIS WILL BREAK THE HARNESS");
        (*real_abort)();
    }

    fuzzywuzzy_ctrl.original_main_fn = main;

    return (*real___libc_start_main)(fuzzywuzzy_main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}

int puts(const char *s) {
    LOAD_GUARD(puts);

    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    return (*real_puts)(s);
}

int printf(const char *format, ...) {
    LOAD_GUARD(vprintf);

    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    va_list list;
    va_start(list, format);
    int res = vprintf(format, list);
    va_end(list);
    return res;
}