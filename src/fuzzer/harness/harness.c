#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <string.h>

#define BUF_SIZE 8192
#define NUM_MMAPS 32
#define NUM_SIGNALS 32

struct mmap_data {
    void *addr;
    size_t len;
};

struct control_data {
    void *ebp;
    void *esp;
    struct mmap_data mmaps[NUM_MMAPS];
    size_t mmap_index;

    bool signals[NUM_SIGNALS];

    void *stack_min;
    void *stack_max;

    void *stack_storage;


};


static void fuzzywuzzy_calc_stack_size();

static void fuzzywuzzy_reset();

static struct control_data *fuzzywuzzy_ctrl = NULL;
static char *buf;

static int (*original_main_fn)(int, char **, char **) = NULL;

int fuzzywuzzy_main(int argc, char **argv, char **environ) {

    if (fuzzywuzzy_ctrl == NULL) {

        //first run
        fuzzywuzzy_ctrl = malloc(sizeof *fuzzywuzzy_ctrl);
        buf = malloc(BUF_SIZE);

        //fuzzywuzzy_ctrl->dlsym_handle = dlopen("libc.so", RTLD_LAZY | RTLD_LOCAL);


        fuzzywuzzy_calc_stack_size();
        printf("stack base: %p, stack top: %p\n", fuzzywuzzy_ctrl->stack_min, fuzzywuzzy_ctrl->stack_max);
        fuzzywuzzy_ctrl->stack_storage = malloc(fuzzywuzzy_ctrl->stack_max - fuzzywuzzy_ctrl->stack_min);


        //
        //memcpy()

    }

    //eax is about to be nuked, who cares what we do to it
    __asm__ (
            "mov eax, dword [ebx+0x44]\n"
            "add eax, 0\n"
            "mov [eax], ebp\n"
            );
    __asm__ (
            "mov eax, dword [ebx+0x44]\n"
            "add eax, 4\n"
            "mov [eax], esp\n"
            );


    printf("ebp=%p esp=%p\n", fuzzywuzzy_ctrl->ebp, fuzzywuzzy_ctrl->esp);

    //restore stack
    //hook libc functions
    //fuzzywuzzy_reset();
    return original_main_fn(argc, argv, environ);

    return 0;
}

static void fuzzywuzzy_reset() {
    for (int i = 1; i < NUM_SIGNALS; i++) {
        if (fuzzywuzzy_ctrl->signals[i]) {
            signal(i, SIG_DFL);
        }
    }

    for (int i = 0; i < fuzzywuzzy_ctrl->mmap_index; i++) {
        if (fuzzywuzzy_ctrl->mmaps[i].addr != NULL) {
            munmap(fuzzywuzzy_ctrl->mmaps[i].addr, fuzzywuzzy_ctrl->mmaps[i].len);
        }
    }

    fuzzywuzzy_ctrl->mmap_index = 0;

}

typedef enum parse_state {
    PARSE_STATE_ADDRS,
    PARSE_STATE_LOOK,
    PARSE_STATE_NAME,
    PARSE_STATE_FAILED,
    PARSE_STATE_DONE,
} parse_state_t;


static void fuzzywuzzy_calc_stack_size() {
    int fd = open("/proc/self/maps", O_RDONLY);


    void *min = NULL;
    void *max = NULL;
    const char *stack_str = "[stack]";


    read(fd, buf, BUF_SIZE); //TODO: BUFFERED READ

    parse_state_t state = PARSE_STATE_ADDRS;
    int marker = 0;
    for (int i = 0; buf[i] != 0; i++) {
        if (buf[i] == '\n') {
            marker = i + 1;
            state = PARSE_STATE_ADDRS;
            continue;
        }
        switch (state) {
            case PARSE_STATE_ADDRS:
                if (buf[i] == '-') {
                    buf[i] = 0;
                    min = (void *) (strtoul(&buf[marker], NULL, 16));
                    buf[i] = '-';
                    marker = i + 1;
                } else if (buf[i] == ' ') {
                    buf[i] = 0;
                    max = (void *) (strtoul(&buf[marker], NULL, 16));
                    buf[i] = ' ';
                    state = PARSE_STATE_LOOK;
                }
                break;
            case PARSE_STATE_LOOK:
                if (buf[i] == '[') {
                    state = PARSE_STATE_NAME;
                }
                break;
            case PARSE_STATE_NAME:
                if (stack_str[i - marker] == 0) {
                    state = PARSE_STATE_DONE;
                } else if (buf[i] != stack_str[i - marker]) {
                    state = PARSE_STATE_FAILED;
                }
                break;
            case PARSE_STATE_FAILED:
                if (buf[i] == '\n') {
                    marker = i + 1;
                    state = PARSE_STATE_ADDRS;
                }
                break;
            case PARSE_STATE_DONE:
                break;
        }

        if (state == PARSE_STATE_DONE) {
            break;
        }
    }


    close(fd);
    fuzzywuzzy_ctrl->stack_min = min;
    fuzzywuzzy_ctrl->stack_max = max;
}


static void *(*real_mmap)(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    *(void **) (&real_mmap) = dlsym(RTLD_NEXT, "mmap");
    void *res = (*real_mmap)(addr, length, prot, flags, fd, offset);

    fuzzywuzzy_ctrl->mmaps[fuzzywuzzy_ctrl->mmap_index++] = (struct mmap_data) {res, length};

    return res;
}


static void (*(*real_signal)(int, void (*func)(int)))(int);

void (*signal(int sig, void (*func)(int)))(int) {
    puts("signal");
    if (!real_signal) {
        *(void **) (&real_signal) = dlsym(RTLD_NEXT, __func__);
    }
    //fuzzywuzzy_ctrl->signals[sig] = true;
    printf("registering signal for %d\n", sig);

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

    original_main_fn = main;


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

void (*real_exit)(int status);

_Noreturn void exit(int status) {
    if (!real_exit) {
        *(void **) (&real_exit) = dlsym(RTLD_NEXT, __func__);
    }
    puts("exit");
    puts("jokes");
    original_main_fn(0, NULL, NULL);

    (*real_exit)(status);
}