#pragma once

#include <bits/types/FILE.h>
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

typedef double    f32;
typedef uintptr_t uptr;
typedef intptr_t  iptr;
typedef uint64_t  u64;
typedef uint32_t  u32;
typedef int64_t   i64;
typedef int32_t   i32;
typedef uint8_t   u8;
typedef int8_t    i8;

#define STRINGIFY(S) STRINGIFY_(S)
#define STRINGIFY_(S) #S
#define CASE(X) case (X):
#define TODO(FEATURE)                                                                   \
    do {                                                                                \
        fprintf(stderr, "TODO: " FEATURE " in " __FILE__ ":" STRINGIFY(__LINE__) "\n"); \
        abort();                                                                        \
    } while (0)
#define UNREACHABLE                                                                 \
    do {                                                                            \
        fprintf(stderr, "should not reach " __FILE__ ":" STRINGIFY(__LINE__) "\n"); \
        abort();                                                                    \
    } while (0)
#define LIST(NODE) NODE,
#define STRING_LIST(NODE) STRINGIFY(NODE),
#define ARR_LEN(ARR) (sizeof(ARR) / sizeof(ARR[0]))
#define EXTENDS(SUPER) SUPER super
#ifndef WJP_DEBUG
#define LOG(...)
#define ASSERT(COND, ...) true
#define NDEBUG
#else
#define LOG(...)                      \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n");        \
    } while (0);
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
#endif

#define swap(X, Y)                     \
    do {                               \
        __auto_type __tmp_x = (X);     \
        (X)                 = (Y);     \
        (Y)                 = __tmp_x; \
    } while (0)
#define max(X, Y)                                    \
    ({                                               \
        __auto_type __tmp_x = (X);                   \
        __auto_type __tmp_y = (Y);                   \
        (__tmp_x > __tmp_y) ? (__tmp_x) : (__tmp_y); \
    })
#define FOPEN(FNAME, F, MODE) \
    for (FILE *F = fopen(FNAME, MODE); F != NULL; fclose(F), F = NULL)

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
#define LIST_REV_ITER(LIST, IT)    \
    for (__auto_type(IT) = (LIST); \
         (IT) != (void *) NULL;    \
         (IT) = (IT)->prev)
#define LIST_ITER(LIST, IT)        \
    for (__auto_type(IT) = (LIST); \
         (IT) != (void *) NULL;    \
         (IT) = (IT)->next)
#define LIST_REMOVE(LIST, FREE, PRED)                \
    do {                                             \
        LIST_ITER((LIST), __it) {                    \
            while (__it->next && PRED(__it->next)) { \
                __auto_type __ptr = __it->next;      \
                __it->next        = __ptr->next;     \
                FREE(__ptr);                         \
            }                                        \
        }                                            \
        if ((LIST) && PRED((LIST))) {                \
            __auto_type __ptr = (LIST);              \
            (LIST)            = __ptr->next;         \
            FREE(__ptr);                             \
        }                                            \
    } while (0)
#define MARKED(NODE) ((NODE)->mark)
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
    F(OP_NEG)

#define SYMS(F) \
    F(SYM_TYP)  \
    F(SYM_VAR)  \
    F(SYM_FUN)

__attribute__((unused)) static inline void *zalloc(u32 size) {
    void *ptr = malloc(size);
    bzero(ptr, size);
    LOG("zalloc %u @ %p", size, ptr);
    return ptr;
}

__attribute__((unused)) static inline void zfree(void *ptr) {
    LOG("zfree @ %p", ptr);
    free(ptr);
}