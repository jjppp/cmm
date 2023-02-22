#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <assert.h>

#define MAX_SYM_LEN 64
#define MAX_SYM 128
#define MAX_DIM 16
#define MAX_CHAR 63
#define SYM_STR_SIZE (sizeof(char) * MAX_SYM_LEN)

typedef double   f32;
typedef uint32_t u32;
typedef int32_t  i32;
typedef uint8_t  u8;
typedef int8_t   i8;

#define STRINGIFY(S) STRINGIFY_(S)
#define STRINGIFY_(S) #S
#define TODO(FEATURE)                                                                   \
    do {                                                                                \
        fprintf(stderr, "TODO: " FEATURE " in " __FILE__ ":" STRINGIFY(__LINE__) "\n"); \
        abort();                                                                        \
    } while (0)
#define ASSERT(COND, ...)                 \
    ({                                    \
        __auto_type __cond = (COND);      \
        if (!__cond) {                    \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, "\n");        \
            assert(__cond);               \
        }                                 \
        1;                                \
    })
#define UNREACHABLE                                                                 \
    do {                                                                            \
        fprintf(stderr, "should not reach " __FILE__ ":" STRINGIFY(__LINE__) "\n"); \
        abort();                                                                    \
    } while (0)
#define LIST(NODE) NODE,
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

#define AST_NODES(F) \
    F(CONS_PROG)     \
    F(CONS_SPEC)     \
    F(CONS_FUN)      \
    /* Stmts */      \
    F(STMT_EXPR)     \
    F(STMT_SCOP)     \
    F(STMT_IFTE)     \
    F(STMT_WHLE)     \
    F(STMT_RET)      \
    /* Exprs */      \
    F(EXPR_CALL)     \
    F(EXPR_IDEN)     \
    F(EXPR_ARR)      \
    F(EXPR_ASS)      \
    F(EXPR_DOT)      \
    F(EXPR_INT)      \
    F(EXPR_FLT)      \
    F(EXPR_BIN)      \
    F(EXPR_UNR)      \
    /* Constructs */ \
    F(DECL_VAR)      \
    F(DECL_TYP)      \
    F(DECL_FUN)

#define AST_NODES_WITH_ARG(F, ARG) \
    F(CONS_PROG, ARG)              \
    F(CONS_SPEC, ARG)              \
    F(CONS_FUN, ARG)               \
    /* Stmts */                    \
    F(STMT_EXPR, ARG)              \
    F(STMT_SCOP, ARG)              \
    F(STMT_IFTE, ARG)              \
    F(STMT_WHLE, ARG)              \
    F(STMT_RET, ARG)               \
    /* Exprs */                    \
    F(EXPR_CALL, ARG)              \
    F(EXPR_IDEN, ARG)              \
    F(EXPR_ARR, ARG)               \
    F(EXPR_ASS, ARG)               \
    F(EXPR_DOT, ARG)               \
    F(EXPR_INT, ARG)               \
    F(EXPR_FLT, ARG)               \
    F(EXPR_BIN, ARG)               \
    F(EXPR_UNR, ARG)               \
    /* Constructs */               \
    F(DECL_VAR, ARG)               \
    F(DECL_FUN, ARG)               \
    F(DECL_TYP, ARG)

#define OPS(F) \
    F(OP_ADD)  \
    F(OP_SUB)  \
    F(OP_MUL)  \
    F(OP_DIV)  \
    F(OP_MOD)  \
    F(OP_AND)  \
    F(OP_LE)   \
    F(OP_LT)   \
    F(OP_GE)   \
    F(OP_GT)   \
    F(OP_NE)   \
    F(OP_EQ)   \
    F(OP_OR)   \
    F(OP_NEG)  \
    F(OP_NOT)

#define LOGIC_OPS(F) \
    F(OP_AND)        \
    F(OP_OR)         \
    F(OP_NOT)

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