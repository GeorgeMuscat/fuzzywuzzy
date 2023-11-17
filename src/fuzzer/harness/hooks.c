#define _GNU_SOURCE
#include "hooks.h"

#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/socket.h>

#include "harness.h"

GEN_WRAPPER(int close, int fd)
GEN_WRAPPER(void free, void_ptr ptr)
GEN_WRAPPER(char_ptr getenv, const_char_ptr name)
GEN_WRAPPER(void_ptr memset, void_ptr str, int c, size_t n)
GEN_WRAPPER(int puts, const_char_ptr s)
GEN_WRAPPER(ssize_t read, int fd, void_ptr buf, size_t count)
GEN_WRAPPER(int socket, int domain, int type, int protocol)
GEN_WRAPPER(char_ptr strcpy, char_ptr dest, const_char_ptr src)
GEN_WRAPPER(size_t strlen, const_char_ptr s)
GEN_WRAPPER(ssize_t write, int fd, const_void_ptr buf, size_t count)

GEN_DEF(void abort)
GEN_DEF(void exit, int status)
GEN_DEF(void_ptr mmap, void_ptr addr, size_t length, int prot, int flags, int fd, off_t offset)
GEN_DEF(int munmap, void_ptr addr, size_t length)
GEN_DEF(int vprintf, const_char_ptr format, va_list ap)


void (*(*fuzzywuzzy_real_signal)(int, void (*func)(int)))(int);
int *(*fuzzywuzzy_real___libc_start_main)(int (*main)(int, char **, char **), int argc, char **ubp_av, void (*init)(void),
                               void (*fini)(void), void (*rtld_fini)(void), void(*stack_end));


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
    LOAD(connect);
    LOAD(memset);
    LOAD(read);
    LOAD(write);
    LOAD(close);
}

_Noreturn void exit(int status) {
    LOAD_GUARD(exit)
    fuzzywuzzy_reset(status);

    REAL(exit, status);

    // this'll never return :)
    for (;;);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    LOAD_GUARD(mmap);


    void *res = REAL(mmap, addr, length, prot, flags, fd, offset);

    fuzzywuzzy_ctrl.mmaps[fuzzywuzzy_ctrl.mmap_index++] = (struct mmap_data){res, length};

    return res;
}

int munmap(void *addr, size_t length) {
    LOAD_GUARD(munmap)

    int res = REAL(munmap, addr, length);

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
    LOAD_GUARD(signal);

    fuzzywuzzy_ctrl.signals[sig] = true;

    return REAL(signal, sig, func);
}

int *__libc_start_main(int (*main)(int, char **, char **), int argc, char **ubp_av, void (*init)(void), void (*fini)(void),
                       void (*rtld_fini)(void), void(*stack_end)) {
    LOAD_GUARD(__libc_start_main);

    if (fuzzywuzzy_ctrl.original_main_fn != NULL) {
        REAL(puts, "WARNING: LIBC START MAIN TWICE, THIS WILL BREAK THE HARNESS");
        REAL(abort);
    }

    fuzzywuzzy_ctrl.original_main_fn = main;

    return REAL(__libc_start_main, fuzzywuzzy_main, argc, ubp_av, init, fini, rtld_fini, stack_end);
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
