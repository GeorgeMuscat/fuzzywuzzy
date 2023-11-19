typedef void* void_ptr;

#define LPAREN (
#define RPAREN )
#define COMMA ,

#define EXPAND(...) __VA_ARGS__

#define SPLIT(OP, D) EXPAND(OP CAT(SPLIT_, D) RPAREN)

#define SPLIT_int LPAREN int COMMA
#define SPLIT_char LPAREN char COMMA
#define SPLIT_float LPAREN float COMMA
#define SPLIT_double LPAREN double COMMA
#define SPLIT_void LPAREN void COMMA
#define SPLIT_void_ptr LPAREN void* COMMA
#define SPLIT_size_t LPAREN size_t COMMA
#define SPLIT_off_t LPAREN off_t COMMA


#define CAT(arg1, arg2)   CAT1(arg1, arg2)
#define CAT1(arg1, arg2)  CAT2(arg1, arg2)
#define CAT2(arg1, arg2)  CAT3(arg1, arg2)
#define CAT3(arg1, arg2)  arg1##arg2

#define FOR_EACH_1(what, x, ...) what(x)
#define FOR_EACH_2(what, x, ...)\
  what(x),\
  FOR_EACH_1(what,  __VA_ARGS__)
#define FOR_EACH_3(what, x, ...)\
  what(x),\
  FOR_EACH_2(what, __VA_ARGS__)
#define FOR_EACH_4(what, x, ...)\
  what(x),\
  FOR_EACH_3(what,  __VA_ARGS__)
#define FOR_EACH_5(what, x, ...)\
  what(x),\
 FOR_EACH_4(what,  __VA_ARGS__)
#define FOR_EACH_6(what, x, ...)\
  what(x),\
  FOR_EACH_5(what,  __VA_ARGS__)
#define FOR_EACH_7(what, x, ...)\
  what(x),\
  FOR_EACH_6(what,  __VA_ARGS__)
#define FOR_EACH_8(what, x, ...)\
  what(x);\
  FOR_EACH_7(what,  __VA_ARGS__),

#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__)
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0

#define FOR_EACH_(N, what, x, ...) CAT(FOR_EACH_, N)(what, x, __VA_ARGS__)
#define FOR_EACH(what, x, ...) FOR_EACH_(FOR_EACH_NARG(x, __VA_ARGS__), what, x, __VA_ARGS__)

#define SELECT_TYPE(TYPE, NAME) TYPE
#define EXTRACT_TYPE(SIG) SPLIT(SELECT_TYPE, SIG)
#define SELECT_NAME(TYPE, NAME) NAME
#define EXTRACT_NAME(SIG) SPLIT(SELECT_NAME, SIG)

#define GEN_WRAPPER(FN_SIG, BLOCK, ...) \
EXTRACT_TYPE(FN_SIG) (CAT(*fuzzywuzzy_real_, EXTRACT_NAME(FN_SIG)))(__VA_ARGS__); \
FN_SIG (__VA_ARGS__) { \
    if (!CAT(fuzzywuzzy_real_, EXTRACT_NAME(FN_SIG))) { \
         *(void **)(&CAT(real_, EXTRACT_NAME(FN_SIG))) = dlsym(RTLD_NEXT, __func__); \
    } \
    return (*CAT(fuzzywuzzy_real_, EXTRACT_NAME(FN_SIG)))(FOR_EACH(EXTRACT_NAME, __VA_ARGS__)); \
}

GEN_WRAPPER(void_ptr mmap, void_ptr addr, size_t length, int prot, int flags, int fd, off_t offset);