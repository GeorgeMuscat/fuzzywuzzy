#pragma once

#include <stdlib.h>

#define LOAD(FN) *(void **)(&CAT(fuzzywuzzy_real_, FN)) = dlsym(RTLD_NEXT, #FN)

#define LOAD_GUARD(FN) \
    if (!CAT(fuzzywuzzy_real_, FN)) {  \
        LOAD(FN);      \
    }


typedef void* void_ptr;
typedef const void *const_void_ptr;
typedef char* char_ptr;
typedef const char* const_char_ptr;

#define LPAREN (
#define RPAREN )
#define COMMA ,

#define EXPAND(...) __VA_ARGS__

#define SPLIT(OP, D) EXPAND(OP CAT(SPLIT_, D) RPAREN)

#define SPLIT_int LPAREN int COMMA
#define SPLIT_char LPAREN char COMMA
#define SPLIT_char_ptr LPAREN char* COMMA
#define SPLIT_const_char_ptr LPAREN const char* COMMA
#define SPLIT_float LPAREN float COMMA
#define SPLIT_double LPAREN double COMMA
#define SPLIT_void LPAREN void COMMA
#define SPLIT_void_ptr LPAREN void* COMMA
#define SPLIT_const_void_ptr LPAREN const void* COMMA
#define SPLIT_size_t LPAREN size_t COMMA
#define SPLIT_ssize_t LPAREN ssize_t COMMA
#define SPLIT_off_t LPAREN off_t COMMA
#define SPLIT_va_list LPAREN va_list COMMA


#define CAT(arg1, arg2)   CAT1(arg1, arg2)
#define CAT1(arg1, arg2)  CAT2(arg1, arg2)
#define CAT2(arg1, arg2)  CAT3(arg1, arg2)
#define CAT3(arg1, arg2)  arg1##arg2


#define FE_0(WHAT)
#define FE_1(WHAT, X) WHAT(X)
#define FE_2(WHAT, X, ...) WHAT(X), FE_1(WHAT, __VA_ARGS__)
#define FE_3(WHAT, X, ...) WHAT(X), FE_2(WHAT, __VA_ARGS__)
#define FE_4(WHAT, X, ...) WHAT(X), FE_3(WHAT, __VA_ARGS__)
#define FE_5(WHAT, X, ...) WHAT(X), FE_4(WHAT, __VA_ARGS__)
#define FE_6(WHAT, X, ...) WHAT(X), FE_5(WHAT, __VA_ARGS__)
#define FE_7(WHAT, X, ...) WHAT(X), FE_6(WHAT, __VA_ARGS__)

#define GET_MACRO(_0,_1,_2,_3,_4,_5,_6,_7, NAME,...) NAME
#define FOR_EACH(action,...) \
  GET_MACRO(_0,__VA_ARGS__,FE_7,FE_6,FE_5,FE_4,FE_3,FE_2,FE_1,FE_0)(action,__VA_ARGS__)

#define SELECT_TYPE(TYPE, NAME) TYPE
#define EXTRACT_TYPE(SIG) SPLIT(SELECT_TYPE, SIG)
#define SELECT_NAME(TYPE, NAME) NAME
#define EXTRACT_NAME(SIG) SPLIT(SELECT_NAME, SIG)

#define GEN_DEF(FN_SIG, ...) \
EXTRACT_TYPE(FN_SIG) (CAT(*fuzzywuzzy_real_, EXTRACT_NAME(FN_SIG)))(__VA_ARGS__);

#define GEN_WRAPPER(FN_SIG, ...) \
GEN_DEF(FN_SIG, __VA_ARGS__)\
FN_SIG (__VA_ARGS__) {\
    LOAD_GUARD(EXTRACT_NAME(FN_SIG));   \
    save_ra();                          \
    fuzzywuzzy_log_libc_call(__func__, ra); \
    return (*CAT(fuzzywuzzy_real_, EXTRACT_NAME(FN_SIG)))(FOR_EACH(EXTRACT_NAME, __VA_ARGS__)); \
}
void fuzzywuzzy_preload_hooks(void);


#define REAL(fn, ...) \
(*fuzzywuzzy_real_##fn)(__VA_ARGS__)

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