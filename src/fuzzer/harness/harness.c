#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>

#define BUF_SIZE 2048
#define NUM_MMAPS 32
#define NUM_SIGNALS 32

#define BASE_MALLOC_ADDRESS (void *)0x10000000


void *notmalloc(size_t size) {
    return NULL;
}

struct mmap_data {
    void *addr;
    size_t len;
};

struct control_data {
    void *ebp;
    char map_str[15];
    void *current_malloc_address;
    char buf[BUF_SIZE];

    struct mmap_data mmaps[NUM_MMAPS];
    size_t mmap_index;


    bool signals[NUM_SIGNALS];

    void *stack_ptr;
};

#define CTRL_DATA ((struct control_data *) (BASE_MALLOC_ADDRESS))


size_t calc_stack_size();
void reset();


int main_replace(int argc, char **argv, char **environ) {
    struct control_data *ctrl = NULL;
    msync((void *) BASE_MALLOC_ADDRESS, sizeof(struct control_data), MS_SYNC);

    if (errno == ENOMEM) {
        //first run
        ctrl = mmap(BASE_MALLOC_ADDRESS, sizeof(struct control_data), PROT_READ | PROT_WRITE,
                    MAP_FIXED_NOREPLACE | MAP_ANONYMOUS, 0, 0);

        if (errno != 0) {
            //printf("fuck you too bye!\n");
        }
        ctrl->current_malloc_address = BASE_MALLOC_ADDRESS + sizeof(struct control_data);


        size_t stack_size = calc_stack_size();
        ctrl->stack_ptr = notmalloc(stack_size);

        //
        //memcpy()

    } else {
        ctrl = CTRL_DATA;
    }


    __asm__ (
            "mov dword ptr [0x10000000], ebp"
            );

    //restore stack
    //hook libc functions
    reset();
}

void reset() {
    struct control_data *ctrl = CTRL_DATA;
    for (int i = 1; i < NUM_SIGNALS; i++) {
        if (ctrl->signals[i]) {
            signal(i, SIG_DFL);
        }
    }

    for (int i = 0; i < ctrl->mmap_index; i++) {
        if (ctrl->mmaps[i].addr != NULL) {
            munmap(ctrl->mmaps[i].addr, ctrl->mmaps[i].len);
        }
    }


}

size_t calc_stack_size() {
    struct control_data *ctrl = CTRL_DATA;

    int map_fd = open(ctrl->map_str, O_RDONLY);

    read(map_fd, ctrl->buf, BUF_SIZE);
}


void *mmap_replace(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    struct control_data *ctrl = CTRL_DATA;

    void *res = mmap(addr, length, prot, flags, fd, offset);

    ctrl->mmaps[ctrl->mmap_index++] = (struct mmap_data) {res, length};

    return res;
}


void (*signal_replace(int sig, void (*func)(int)))(int) {
    struct control_data *ctrl = CTRL_DATA;

    ctrl->signals[sig] = true;

    //return signal(sig, func);
}

int printf_replace(const char *format, ...) {
    va_list list;
    va_start(list, format);
    //int res = vprintf(format, list);
    int res = 0;
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
    //int res = vsprintf(str, format, list);
    int res = 0;
    va_end(list);
    return res;
}

int snprintf_replace(char *str, size_t size, const char *format, ...) {
    va_list list;
    va_start(list, format);
    //int res = vsnprintf(str, size, format, list);
    int res = 0;
    va_end(list);
    return res;
}