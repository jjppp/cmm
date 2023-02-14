#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <assert.h>

typedef double   f32;
typedef uint32_t u32;
typedef int32_t  i32;
typedef uint8_t  u8;
typedef int8_t   i8;

#define STRINGIFY(S) STRINGIFY_(S)
#define STRINGIFY_(S) #S
#define TODO                                                               \
    {                                                                      \
        fprintf(stderr, "TODO in " __FILE__ ":" STRINGIFY(__LINE__) "\n"); \
        abort();                                                           \
    }                                                                      \
    while (0)
#define ASSERT(COND, ...)             \
    if (!(COND)) {                    \
        fprintf(stderr, __VA_ARGS__); \
        assert(COND);                 \
    }
#define UNREACHABLE                                                                 \
    {                                                                               \
        fprintf(stderr, "should not reach " __FILE__ ":" STRINGIFY(__LINE__) "\n"); \
        abort();                                                                    \
    }                                                                               \
    while (0)
#define LIST(NODE) NODE,
#define ARR_LEN(ARR) (sizeof(ARR) / sizeof(ARR[0]))
#define EXTENDS(SUPER) SUPER super
#ifdef NDEBUG
#define LOG(...)
#else
#define LOG(...) fprintf(stderr, __VA_ARGS__);
#endif

#define AST_NODES(F) \
    F(CONS_PROG)     \
    /* Stmts */      \
    F(STMT_EXPR)     \
    F(STMT_SCOP)     \
    F(STMT_IFTE)     \
    F(STMT_WHLE)     \
    F(STMT_RET)      \
    /* Exprs */      \
    F(EXPR_CALL)     \
    F(EXPR_IDEN)     \
    F(EXPR_ASS)      \
    F(EXPR_DOT)      \
    F(EXPR_INT)      \
    F(EXPR_FLT)      \
    F(EXPR_BIN)      \
    F(EXPR_UNR)      \
    /* Constructs */ \
    F(DECL_VAR)      \
    F(DECL_FUN)

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

static inline void *zalloc(u32 size) {
    void *ptr = malloc(size);
    bzero(ptr, size);
    return ptr;
}