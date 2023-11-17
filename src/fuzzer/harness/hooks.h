#pragma once

#include <stdlib.h>

#define LOAD(FN) *(void **)(&real_##FN) = dlsym(RTLD_NEXT, #FN)

#define LOAD_GUARD(FN) \
    if (!real_##FN) {  \
        LOAD(FN);      \
    }

void fuzzywuzzy_preload_hooks(void);

/*void *(*real_malloc)(size_t size);
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

// int (*real_socket)(int domain, int type, int protocol);

// int socket(int domain, int type, int protocol) {
//     if (!real_socket) {
//         *(void **)(&real_socket) = dlsym(RTLD_NEXT, __func__);
//     }

//     return (*real_socket)(domain, type, protocol);
// }

// int fuzzywuzzy_socket(int domain, int type, int protocol) {
//     if (!real_socket) {
//         *(void **)(&real_socket) = dlsym(RTLD_NEXT, __func__);
//     }

//     return (*real_socket)(domain, type, protocol);
// }

// void (*real_abort)(void);

// void abort(void) {
//     if (!real_abort) {
//         *(void **)(&real_abort) = dlsym(RTLD_NEXT, __func__);
//     }

//     // needs to catch the abort
//     (*real_abort)();
// }

// void fuzzywuzzy_abort(void) {
//     if (!real_abort) {
//         *(void **)(&real_abort) = dlsym(RTLD_NEXT, __func__);
//     }

//     (*real_abort)();
// }