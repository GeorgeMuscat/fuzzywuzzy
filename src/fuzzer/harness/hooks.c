#define _GNU_SOURCE
#include "hooks.h"

#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/socket.h>
#include <ucontext.h>

#include "harness.h"

GEN_WRAPPER(double atof, const_char_ptr s)
GEN_WRAPPER(int atoi, const_char_ptr s)
GEN_WRAPPER(long atol, const_char_ptr s)
GEN_WRAPPER(void_ptr calloc, size_t num, size_t size)
GEN_WRAPPER(int close, int fd)
GEN_WRAPPER(int connect, int sockfd, const_sockaddr_ptr addr, socklen_t addrlen)
GEN_WRAPPER(int fgetc, FILE_ptr stream)
GEN_WRAPPER(char_ptr fgets, char_ptr str, int n, FILE_ptr stream)
GEN_WRAPPER(int fclose, FILE_ptr stream)
GEN_WRAPPER(int fflush, FILE_ptr stream)
GEN_WRAPPER(size_t fread, void_ptr ptr, size_t size, size_t nmemb, FILE_ptr stream)
GEN_WRAPPER(void free, void_ptr ptr)
GEN_WRAPPER(int getc, FILE_ptr stream)
GEN_WRAPPER(char_ptr getenv, const_char_ptr name)
GEN_WRAPPER(char_ptr gets, char_ptr buffer)
GEN_WRAPPER(void_ptr malloc, size_t size)
GEN_WRAPPER(size_t malloc_usable_size, void_ptr ptr)
GEN_WRAPPER(int memcmp, const_void_ptr buf1, const_void_ptr buf2, size_t count)
GEN_WRAPPER(void_ptr memcpy, void_ptr dest, const_void_ptr src, size_t n)
GEN_WRAPPER(void_ptr memmove, void_ptr dest, const_void_ptr src, size_t count)
GEN_WRAPPER(void_ptr memset, void_ptr str, int c, size_t n)
GEN_WRAPPER(int open, const_char_ptr pathname, int flags)
GEN_WRAPPER(void perror, const_char_ptr string)
GEN_WRAPPER(int putc, int c, FILE_ptr stream)
GEN_WRAPPER(int putchar, int c)
GEN_WRAPPER(int puts, const_char_ptr s)
GEN_WRAPPER(ssize_t read, int fd, void_ptr buf, size_t count)
GEN_WRAPPER(void_ptr realloc, void_ptr ptr, size_t size)
GEN_WRAPPER(int socket, int domain, int type, int protocol)
GEN_WRAPPER(int strncasecmp, const_char_ptr string1, const_char_ptr string2, size_t count)
GEN_WRAPPER(char_ptr strcat, char_ptr dest, const_char_ptr src)
GEN_WRAPPER(char_ptr strchr, const_char_ptr str, int c)
GEN_WRAPPER(char_ptr strcpy, char_ptr dest, const_char_ptr src)
GEN_WRAPPER(size_t strcspn, const_char_ptr string1, const_char_ptr string2)
GEN_WRAPPER(size_t strlen, const_char_ptr s)
GEN_WRAPPER(char_ptr strncat, char_ptr string1, const_char_ptr string2, size_t count)
GEN_WRAPPER(int strncmp, const_char_ptr s1, const_char_ptr s2, size_t n)
GEN_WRAPPER(char_ptr strncpy, char_ptr string1, const_char_ptr string2, size_t count)
GEN_WRAPPER(char_ptr strstr, const_char_ptr string1, const_char_ptr string2)
GEN_WRAPPER(unsigned_long strtoul, const_char_ptr nptr, char_ptr_ptr endptr, int base)
GEN_WRAPPER(char_ptr strtok, char_ptr string1, const_char_ptr string2)
GEN_WRAPPER(long strtol, const_char_ptr nptr, char_ptr_ptr endptr, int base)
GEN_WRAPPER(int system, const_char_ptr command)
GEN_WRAPPER(int tolower, int c)
GEN_WRAPPER(int toupper, int c)
GEN_WRAPPER(ssize_t write, int fd, const_void_ptr buf, size_t count)

GEN_WRAPPERNOARG(int getchar)

GEN_DEF(void abort)
GEN_DEF(void assert, int x)
GEN_DEF(void exit, int status)
GEN_DEF(void_ptr mmap, void_ptr addr, size_t length, int prot, int flags, int fd, off_t offset)
GEN_DEF(int munmap, void_ptr addr, size_t length)
GEN_DEF(int vscanf, const_char_ptr format, va_list ap);
GEN_DEF(int vfscanf, FILE_ptr f, const_char_ptr format, va_list ap);
GEN_DEF(int vsscanf, const_char_ptr s, const_char_ptr format, va_list ap);
GEN_DEF(int vfprintf, FILE_ptr f, const_char_ptr format, va_list ap)
GEN_DEF(int vprintf, const_char_ptr format, va_list ap)
GEN_DEF(int vsprintf, char_ptr s, const_char_ptr format, va_list ap)
GEN_DEF(int vsnprintf, char_ptr s, size_t n, const_char_ptr format, va_list ap)

GEN_DEF(void __stack_chk_fail)

void (*(*fuzzywuzzy_real_signal)(int, void (*func)(int)))(int);
int *(*fuzzywuzzy_real___libc_start_main)(int (*main)(int, char **, char **), int argc, char **ubp_av, void (*init)(void),
                                          void (*fini)(void), void (*rtld_fini)(void), void(*stack_end));

extern struct control_data fuzzywuzzy_ctrl;

void fuzzywuzzy_preload_hooks(void) {
    LOAD(close);
    LOAD(connect);
    LOAD(free);
    LOAD(fgets);
    LOAD(getenv);
    LOAD(malloc);
    LOAD(malloc_usable_size);
    LOAD(memset);
    LOAD(open);
    LOAD(puts);
    LOAD(read);
    LOAD(socket);
    LOAD(strchr);
    LOAD(strcpy);
    LOAD(strlen);
    LOAD(strncmp);
    LOAD(strtoul);
    LOAD(write);


    LOAD(abort);
    LOAD(exit);
    LOAD(mmap);
    LOAD(munmap);
    LOAD(vscanf);
    LOAD(vfscanf);
    LOAD(vsscanf);
    LOAD(vfprintf);
    LOAD(vprintf);
    LOAD(vsprintf);
    LOAD(vsnprintf);

    LOAD(signal);
}

void assert(int x) {
    LOAD_GUARD(assert)
    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    if (x) {
        fuzzywuzzy_reset(1);
    } else {
        return;
    }
}

_Noreturn void abort() {
    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    fuzzywuzzy_reset(-6);

    // this'll never return :)
    for (;;);
}

_Noreturn void exit(int status) {
    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    fuzzywuzzy_reset(status);

    // this'll never return :)
    for (;;);
}

void __stack_chk_fail() {
    LOAD_GUARD(__stack_chk_fail)
    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);
    //REAL(puts)("crash");

    return REAL(__stack_chk_fail)();
}


void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    void *res = REAL(mmap)(addr, length, prot, flags, fd, offset);

    fuzzywuzzy_ctrl.mmaps[fuzzywuzzy_ctrl.mmap_index++] = (struct mmap_data){res, length};

    return res;
}

int munmap(void *addr, size_t length) {
    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    int res = REAL(munmap)(addr, length);

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
    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    fuzzywuzzy_ctrl.signals[sig] = func;

    return REAL(signal)(sig, func);
}

int *__libc_start_main(int (*main)(int, char **, char **), int argc, char **ubp_av, void (*init)(void), void (*fini)(void),
                       void (*rtld_fini)(void), void(*stack_end)) {
    if (fuzzywuzzy_ctrl.original_main_fn != NULL) {
        LOAD(puts);
        LOAD(abort);
        REAL(puts)("WARNING: LIBC START MAIN CALLED TWICE, THIS WILL BREAK THE HARNESS");
        REAL(abort)();
    }

    fuzzywuzzy_ctrl.original_main_fn = main;

    LOAD(__libc_start_main);
    return REAL(__libc_start_main)(fuzzywuzzy_main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}

int fprintf(FILE *f, const char *format, ...) {
    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    va_list list;
    va_start(list, format);
    int res = REAL(vfprintf)(f, format, list);
    va_end(list);
    return res;
}

int printf(const char *format, ...) {
    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    va_list list;
    va_start(list, format);
    int res = REAL(vprintf)(format, list);
    va_end(list);
    return res;
}

int sprintf(char *s, const char *format, ...) {
    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    va_list list;
    va_start(list, format);
    int res = REAL(vsprintf)(s, format, list);
    va_end(list);
    return res;
}

int snprintf(char *s, size_t n, const char *format, ...) {
    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    va_list list;
    va_start(list, format);
    int res = REAL(vsnprintf)(s, n, format, list);
    va_end(list);
    return res;
}


int scanf(const char *format, ...) {
    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    va_list list;
    va_start(list, format);
    int res = REAL(vscanf)(format, list);
    va_end(list);
    return res;
}

int sscanf(const char *s, const char *format, ...) {
    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    va_list list;
    va_start(list, format);
    int res = REAL(vsscanf)(s, format, list);
    va_end(list);
    return res;
}

int fscanf(FILE *f, const char *format, ...) {
    save_ra();
    fuzzywuzzy_log_libc_call(__func__, ra);

    va_list list;
    va_start(list, format);
    int res = REAL(vfscanf)(f, format, list);
    va_end(list);
    return res;
}

