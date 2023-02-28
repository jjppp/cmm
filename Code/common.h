#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <assert.h>

#define MAX_SYM_LEN 64
#define MAX_SYM 8192
#define MAX_DIM 64
#define MAX_CHAR 63
#define SYM_STR_SIZE (sizeof(char) * MAX_SYM_LEN)

typedef double   f32;
typedef uint32_t u32;
typedef int32_t  i32;
typedef uint8_t  u8;
typedef int8_t   i8;

#define STRINGIFY(S) STRINGIFY_(S)
#define STRINGIFY_(S) #S
#define CASE(X) case (X):
#define TODO(FEATURE)                                                                   \
    do {                                                                                \
        fprintf(stderr, "TODO: " FEATURE " in " __FILE__ ":" STRINGIFY(__LINE__) "\n"); \
        abort();                                                                        \
    } while (0)
#define ASSERT(COND, ...)                                           \
    ({                                                              \
        __auto_type __cond = (COND);                                \
        if (!__cond) {                                              \
            fprintf(stderr, __FILE__ ":" STRINGIFY(__LINE__) "\n"); \
            fprintf(stderr, __VA_ARGS__);                           \
            fprintf(stderr, "\n");                                  \
            assert(__cond);                                         \
        }                                                           \
        1;                                                          \
    })
#define UNREACHABLE                                                                 \
    do {                                                                            \
        fprintf(stderr, "should not reach " __FILE__ ":" STRINGIFY(__LINE__) "\n"); \
        abort();                                                                    \
    } while (0)
#define LIST(NODE) NODE,
#define STRING_LIST(NODE) STRINGIFY(NODE),
#define ARR_LEN(ARR) (sizeof(ARR) / sizeof(ARR[0]))
#define EXTENDS(SUPER) SUPER super
#ifdef NDEBUG
#define LOG(...)
#else
#define LOG(...)                      \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n");        \
    } while (0);
#endif

#define swap(X, Y)                     \
    do {                               \
        __auto_type __tmp_x = (X);     \
        (X)                 = (Y);     \
        (Y)                 = __tmp_x; \
    } while (0)

/* refcount ptr. obj must extend struct shared. */
#define POINTS_TO(PTR, OBJ)                                    \
    do {                                                       \
        shared *__tmp = (void *) (OBJ);                        \
        (PTR)         = (void *) __tmp;                        \
        if (__tmp == NULL) break;                              \
        __tmp->nref++;                                         \
        LOG(STRINGIFY(PTR) " -> %p, #%u", (PTR), __tmp->nref); \
    } while (0)

#define POINTS_FREE(PTR, DESTRUCTOR)                            \
    do {                                                        \
        shared *__tmp = (void *) (PTR);                         \
        (PTR)         = NULL;                                   \
        if (__tmp == NULL) break;                               \
        __tmp->nref--;                                          \
        LOG(STRINGIFY(PTR) " x-> %p, #%u", __tmp, __tmp->nref); \
        if (__tmp->nref == 0) {                                 \
            DESTRUCTOR((void *) __tmp);                         \
        }                                                       \
    } while (0)

typedef struct shared shared;

struct shared {
    u32 nref;
};

#define LIST_APPEND(LIST, NODE)           \
    do {                                  \
        __auto_type __node = (NODE);      \
        if ((LIST) == NULL) {             \
            (LIST) = __node;              \
            break;                        \
        }                                 \
        for (__auto_type __list = (LIST); \
             __list != (void *) NULL;     \
             __list = __list->next) {     \
            if (__list->next == NULL) {   \
                __list->next = __node;    \
                break;                    \
            }                             \
        }                                 \
    } while (0)
#define LIST_LENGTH(LIST)                 \
    ({                                    \
        u32 __length = 0;                 \
        for (__auto_type __list = (LIST); \
             __list != (void *) NULL;     \
             __list = __list->next) {     \
            __length++;                   \
        }                                 \
        __length;                         \
    })
#define LIST_ITER(LIST, IT)        \
    for (__auto_type(IT) = (LIST); \
         (IT) != (void *) NULL;    \
         (IT) = (IT)->next)
#define LIST_FOREACH(LIST, FUN) \
    LIST_ITER(LIST, __it)       \
    FUN(__it);

#define OPS(F)   \
    ARITH_OPS(F) \
    LOGIC_OPS(F) \
    REL_OPS(F)

#define LOGIC_OPS(F) \
    F(OP_AND)        \
    F(OP_OR)         \
    F(OP_NOT)

#define REL_OPS(F) \
    F(OP_LT)       \
    F(OP_LE)       \
    F(OP_GT)       \
    F(OP_GE)       \
    F(OP_EQ)       \
    F(OP_NE)

#define ARITH_OPS(F) \
    F(OP_ADD)        \
    F(OP_SUB)        \
    F(OP_MUL)        \
    F(OP_DIV)        \
    F(OP_MOD)        \
    F(OP_NEG)

#define SYMS(F) \
    F(SYM_TYP)  \
    F(SYM_VAR)  \
    F(SYM_FUN)

static inline void *zalloc(u32 size) {
    void *ptr = malloc(size);
    bzero(ptr, size);
    LOG("zalloc %u @ %p", size, ptr);
    return ptr;
}

static inline void zfree(void *ptr) {
    LOG("zfree @ %p", ptr);
    free(ptr);
}