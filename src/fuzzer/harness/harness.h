#pragma once

#include <dlfcn.h>

#define BUF_SIZE 8192
#define NUM_MMAPS 32
#define NUM_SIGNALS 32
#define NUM_REGIONS 16

#define MMAP_BASE 0x20000000

#define CTRL_OFFSET "0xbc" // if everything stop working, check this

#define save_ra() \
void *ra = NULL;\
__asm__(\
        "mov %[asm_ra], [ebp+4]\n"\
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

    bool signals[NUM_SIGNALS];

    size_t mmap_index;
    struct mmap_data mmaps[NUM_MMAPS];

    int (*original_main_fn)(int, char **, char **);

    char buf[BUF_SIZE];
    int *dummy_malloc;
};

struct control_data fuzzywuzzy_ctrl = {0};

int fuzzywuzzy_main(int argc, char **argv, char **environ);
void fuzzywuzzy_read_mmap();

_Noreturn void fuzzywuzzy_reset();

void (*real_free)(void *ptr);

void free(void *ptr) {
    if (!real_free) {
        *(void **) (&real_free) = dlsym(RTLD_NEXT, __func__);
    }
    (*real_free)(ptr);
}

void (*real_exit)(int status);

_Noreturn void exit(int status) {
    if (!real_exit) {
        *(void **) (&real_exit) = dlsym(RTLD_NEXT, __func__);
    }
    fuzzywuzzy_reset();

    (*real_exit)(status);
}

static void *(*real_mmap)(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    if (!real_mmap) {
        *(void **) (&real_mmap) = dlsym(RTLD_NEXT, __func__);
    }

    void *res = (*real_mmap)(addr, length, prot, flags, fd, offset);

    fuzzywuzzy_ctrl.mmaps[fuzzywuzzy_ctrl.mmap_index++] = (struct mmap_data) {res, length};

    return res;
}

void *fuzzywuzzy_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    if (!real_mmap) {
        *(void **) (&real_mmap) = dlsym(RTLD_NEXT, "mmap");
    }

    return (*real_mmap)(addr, length, prot, flags, fd, offset);
}

static int (*real_munmap)(void *addr, size_t length);

int munmap(void *addr, size_t length) {
    if (!real_munmap) {
        *(void **) (&real_munmap) = dlsym(RTLD_NEXT, __func__);
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


static void (*(*real_signal)(int, void (*func)(int)))(int);

void (*signal(int sig, void (*func)(int)))(int) {
    if (!real_signal) {
        *(void **) (&real_signal) = dlsym(RTLD_NEXT, __func__);
    }
    fuzzywuzzy_ctrl.signals[sig] = true;

    save_ra();

    return (*real_signal)(sig, func);
}

static int *(*real_libc_start_main)(int (*main)(int, char **, char **), int argc, char **ubp_av, void (*init)(void),
                                    void (*fini)(void), void (*rtld_fini)(void), void (*stack_end));

int *
__libc_start_main(int (*main)(int, char **, char **), int argc, char **ubp_av, void (*init)(void), void (*fini)(void),
                  void (*rtld_fini)(void), void (*stack_end)) {

    if (!real_libc_start_main) {
        *(void **) (&real_libc_start_main) = dlsym(RTLD_NEXT, __func__);
    }

    if (fuzzywuzzy_ctrl.original_main_fn != NULL) {
        puts("WARNING: LIBC START MAIN TWICE, THIS WILL BREAK THE HARNESS");
        abort();
    }

    fuzzywuzzy_ctrl.original_main_fn = main;


    return (*real_libc_start_main)(fuzzywuzzy_main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}


/*static void *(*real_malloc)(size_t size);
void *malloc(size_t size) {
    puts("malloc");
    if (!real_malloc) {
        *(void **)(&real_malloc) = dlsym(RTLD_NEXT __func__);
    }
    void *res = (*real_malloc)(size);

    puts("malloc size %x @ %x");
    return res;
}*/

/*
int printf_replace(const char *format, ...) {
    va_list list;
    va_start(list, format);
    int res = vprintf(format, list);
    va_end(list);
    return res;
}

int fprintf_replace(FILE *stream, const char *format, ...) {
    va_list list;
    va_start(list, format);
    //int res = vfprintf(stream, format, list);
    int res = 0;
    va_end(list);
    return res;
}

int sprintf_replace(char *str, const char *format, ...) {
    va_list list;
    va_start(list, format);
    int res = vsprintf(str, format, list);
    va_end(list);
    return res;
}

int snprintf_replace(char *str, size_t size, const char *format, ...) {
    va_list list;
    va_start(list, format);
    int res = vsnprintf(str, size, format, list);
    va_end(list);
    return res;
}

*/


