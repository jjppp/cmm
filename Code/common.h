#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

typedef float    f32;
typedef uint32_t u32;
typedef int32_t  i32;
typedef uint8_t  u8;
typedef int8_t   i8;

#define STRINGIFY(S) \
    STRINGIFY_(S)

#define STRINGIFY_(S) #S

#define ARR_LEN(ARR) \
    (sizeof(ARR) / sizeof(ARR[0]))

#define EXTENDS(SUPER) \
    SUPER super

#define AST_NODES(F) \
    F(PROGRAM)       \
    /* Stmts */      \
    F(STMT_SCOP)     \
    F(STMT_IFTE)     \
    F(STMT_WHLE)     \
    F(STMT_RET)      \
    F(STMT_DOT)      \
    F(STMT_ASS)      \
    /* Exprs */      \
    F(EXPR_IDEN)     \
    F(EXPR_INT)      \
    F(EXPR_FLT)      \
    F(EXPR_BIN)      \
    F(EXPR_UNR)      \
    /* Constructs */ \
    F(DECL_VAR)      \
    F(DECL_FUN)

static inline void *zalloc(u32 size) {
    void *ptr = malloc(size);
    bzero(ptr, size);
    return ptr;
}

#endif