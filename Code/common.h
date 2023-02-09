#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

typedef uint32_t u32;
typedef int32_t  i32;
typedef uint8_t  u8;
typedef int8_t   i8;

#define ARR_LEN(ARR) \
    (sizeof(ARR) / sizeof(ARR[0]))

#define AST_NODES(F)   \
    F(TYPE)            \
    F(ID)              \
    F(STRUCT)          \
    F(IF)              \
    F(ELSE)            \
    F(RETURN)          \
    F(WHILE)           \
    F(SEMI)            \
    F(COMMA)           \
    F(DOT)             \
    F(ASSIGNOP)        \
    F(RELOP)           \
    F(LE)              \
    F(GE)              \
    F(LT)              \
    F(GT)              \
    F(EQ)              \
    F(NE)              \
    F(ADD)             \
    F(SUB)             \
    F(MUL)             \
    F(DIV)             \
    F(AND)             \
    F(OR)              \
    F(NOT)             \
    F(LP)              \
    F(RP)              \
    F(LB)              \
    F(RB)              \
    F(LC)              \
    F(RC)              \
    F(Program)         \
    F(ExtDefList)      \
    F(ExtDef)          \
    F(ExtDecList)      \
    F(Specifier)       \
    F(StructSpecifier) \
    F(OptTag)          \
    F(Tag)             \
    F(VarDec)          \
    F(FunDec)          \
    F(ParamDec)        \
    F(Dec)             \
    F(DecList)         \
    F(VarList)         \
    F(CompSt)          \
    F(StmtList)        \
    F(Stmt)            \
    F(DefList)         \
    F(Def)             \
    F(EXP)             \
    F(Arg)

static inline void *zalloc(u32 size) {
    void *ptr = malloc(size);
    bzero(ptr, size);
    return ptr;
}

#endif