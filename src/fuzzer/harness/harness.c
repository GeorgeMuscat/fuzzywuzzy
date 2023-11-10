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
#define NUM_REGIONS 16

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

    size_t mmap_index;

    void *stack_min;
    void *stack_max;

    void *stack_storage;
    size_t stack_size;
    struct memory_region writable[NUM_REGIONS]; // todo should we malloc this? i really dont want to touch re-alloc in here, fragmentation is BAD.
    size_t writable_index;
};

static void fuzzywuzzy_read_mmap();

static void fuzzywuzzy_reset();
struct mmap_data mmaps[NUM_MMAPS];
static bool signals[NUM_SIGNALS];

static struct control_data *fuzzywuzzy_ctrl = NULL;

static char *buf;

static int (*original_main_fn)(int, char **, char **) = NULL;


int fuzzywuzzy_main(int argc, char **argv, char **environ) {
    if (fuzzywuzzy_ctrl == NULL) {

        //first run
        fuzzywuzzy_ctrl = malloc(sizeof *fuzzywuzzy_ctrl);
        buf = malloc(BUF_SIZE);

        //fuzzywuzzy_ctrl->dlsym_handle = dlopen("libc.so", RTLD_LAZY | RTLD_LOCAL);


        fuzzywuzzy_read_mmap();
        printf("stack base: %p, stack top: %p\n", fuzzywuzzy_ctrl->stack_min, fuzzywuzzy_ctrl->stack_max);
        fuzzywuzzy_ctrl->stack_size = fuzzywuzzy_ctrl->stack_max - fuzzywuzzy_ctrl->stack_min;
        fuzzywuzzy_ctrl->stack_storage = malloc(fuzzywuzzy_ctrl->stack_size);

        //
        //memcpy()
    }
    //fuzzywuzzy_ctrl->ra = &&saved;

    /*__asm__("mov eax, dword [ebx+0x18c]\n"
            "mov [eax + 0x10], esp\n");*/

    // DO NOT CHANGE ANYTHING BELOW THIS LINE IN THE MAIN FUNCTION. YOU CANNOT CALL C CODE BELOW HERE.

    //eax is about to be nuked, who cares what we do to it
    //save reg
    __asm__ (
            "mov eax, dword [ebx+0x18c]\n"
            "mov [eax + 0x00], eax\n"
            "mov [eax + 0x04], ecx\n"
            "mov [eax + 0x08], edx\n"
            "mov [eax + 0x0c], ebx\n"
            "mov [eax + 0x10], esp\n"
            "mov [eax + 0x14], ebp\n"
            "mov [eax + 0x18], esi\n"
            "mov [eax + 0x1c], edi\n"
            );

    // save stack
    __asm__(
            "mov eax, dword [ebx+0x18c]\n"
            "mov esi, 0\n"
            "fuzzywuzzy_save_stack_loop:\n"
            "mov edx, [eax + 0x2c]\n"   // edx = &stack_storage[0]
            "lea edx, [edx + esi]\n"    // edx = &stack_storage[i]
            "mov ecx, [eax + 0x24]\n"   // ecx = &stack_min[0]
            "mov ecx, [ecx + esi]\n"    // ecx = stack_min[i]
            "mov [edx], ecx\n"          // stack_storage[i] = stack_min[i]
            "mov edx, [eax + 0x30]\n"   // edx = stack_size
            "add esi, 4\n"              // i += 1
            "cmp esi, edx\n"            // cmp i + 1, stack_size
            "jne fuzzywuzzy_save_stack_loop\n"
            );

    // restore registers
    __asm__(
            "mov eax, dword [ebx+0x18c]\n"
            "mov eax, [eax + 0x00]\n"
            "mov ecx, [eax + 0x04]\n"
            "mov edx, [eax + 0x08]\n"
            "mov ebx, [eax + 0x0c]\n"
            "mov esp, [eax + 0x10]\n"
            "mov ebp, [eax + 0x14]\n"
            "mov esi, [eax + 0x18]\n"
            "mov edi, [eax + 0x1c]\n"
            );

    printf("%p, eax = %p, ebx = %p, ebp = %p, esp = %p\n", fuzzywuzzy_ctrl, fuzzywuzzy_ctrl->eax, fuzzywuzzy_ctrl->ebx, fuzzywuzzy_ctrl->ebp, fuzzywuzzy_ctrl->esp);

    __asm__("saved:\n");

    return original_main_fn(argc, argv, environ);
}

static void fuzzywuzzy_reset() {
    for (int i = 1; i < NUM_SIGNALS; i++) {
        if (signals[i]) {
            signal(i, SIG_DFL);
        }
    }

    for (int i = 0; i < fuzzywuzzy_ctrl->mmap_index; i++) {
        if (mmaps[i].addr != NULL) {
            munmap(mmaps[i].addr, mmaps[i].len);
        }
    }



    fuzzywuzzy_ctrl->mmap_index = 0;
    // nuke stack
    __asm__(
            "mov eax, dword [ebx+0x18c]\n"
            "mov esi, 0\n"
            "fuzzywuzzy_reset_stack_loop:\n"
            "mov edx, [eax + 0x2c]\n"   // edx = &stack_storage[0]
            "mov edx, [edx + esi]\n"    // edx = stack_storage[i]
            "mov ecx, [eax + 0x24]\n"   // ecx = &stack_min[0]
            "lea ecx, [ecx + esi]\n"    // ecx = &stack_min[i]
            "mov [ecx], edx\n"          // stack_min[i] = stack_storage[i]
            "mov edx, [eax + 0x30]\n"   // edx = stack_size
            "add esi, 4\n"              // i += 1
            "cmp esi, edx\n"            // cmp i + 1, stack_size
            "jne fuzzywuzzy_reset_stack_loop\n"
            );

    __asm__(
            "mov eax, dword [ebx+0x18c]\n"
            "mov eax, [eax + 0x00]\n"
            "mov ecx, [eax + 0x04]\n"
            "mov edx, [eax + 0x08]\n"
            "mov ebx, [eax + 0x0c]\n"
            "mov esp, [eax + 0x10]\n"
            "mov ebp, [eax + 0x14]\n"
            "mov esi, [eax + 0x18]\n"
            "mov edi, [eax + 0x1c]\n"
            );

    __asm__("jmp saved\n");
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


static void fuzzywuzzy_read_mmap() {
    int fd = open("/proc/self/maps", O_RDONLY);

    void *base = NULL;
    void *top = NULL;
    char prot[5] = {0};
    size_t offset = 0;
    const char *stack_str = "[stack]";
    const char *harness_str = "harness.so";
    int name_start = 0;
    int name_end = 0;


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
            for (int j = name_start; j <= name_end; j--) {
                if (buf[name_start] != harness_str[j - name_start]) {
                    should_save = false;
                }
            }
            if (prot[1] != 'w') {
                should_save = false;
            }

            if (should_save) {
                fuzzywuzzy_ctrl->writable[fuzzywuzzy_ctrl->writable_index++] = (struct memory_region){base, top, top - base, malloc(top - base)};
            }

            state = PARSE_STATE_ADDRS;
        }
    }


    close(fd);
}


static void *(*real_mmap)(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    *(void **) (&real_mmap) = dlsym(RTLD_NEXT, "mmap");
    void *res = (*real_mmap)(addr, length, prot, flags, fd, offset);

    mmaps[fuzzywuzzy_ctrl->mmap_index++] = (struct mmap_data) {res, length};

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

    if (original_main_fn != NULL) {
        puts("WARNING: LIBC START MAIN TWICE, THIS WILL BREAK THE HARNESS");
        abort();
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