#pragma once

#include "ast.h"
#include "common.h"
#include "symtab.h"

#define IR(F)    \
    F(IR_LABEL)  \
    F(IR_ASSIGN) \
    F(IR_BINARY) \
    F(IR_UNARY)  \
    F(IR_DREF)   \
    F(IR_LOAD)   \
    F(IR_STORE)  \
    F(IR_GOTO)   \
    F(IR_BRANCH) \
    F(IR_RETURN) \
    F(IR_DEC)    \
    F(IR_ARG)    \
    F(IR_CALL)   \
    F(IR_PARAM)  \
    F(IR_READ)   \
    F(IR_WRITE)

#define IR_WITH_ARG(F, ARG) \
    F(IR_LABEL, ARG)        \
    F(IR_ASSIGN, ARG)       \
    F(IR_BINARY, ARG)       \
    F(IR_UNARY, ARG)        \
    F(IR_DREF, ARG)         \
    F(IR_LOAD, ARG)         \
    F(IR_STORE, ARG)        \
    F(IR_GOTO, ARG)         \
    F(IR_BRANCH, ARG)       \
    F(IR_RETURN, ARG)       \
    F(IR_DEC, ARG)          \
    F(IR_ARG, ARG)          \
    F(IR_CALL, ARG)         \
    F(IR_PARAM, ARG)        \
    F(IR_READ, ARG)         \
    F(IR_WRITE, ARG)

typedef enum {
    IR(LIST)
} ir_kind_t;

typedef struct block_t  block_t;
typedef struct IR_list  ir_list;
typedef struct IR_fun_t ir_fun_t;
typedef struct IR_t     IR_t;
typedef struct oprd_t   oprd_t;

typedef enum {
    OPRD_LIT,
    OPRD_VAR,
    OPRD_PTR,
} oprd_kind_t;

struct oprd_t {
    const char *name;
    oprd_kind_t kind;
    u32         val;
};

struct IR_t {
    EXTENDS(shared);
    char      str[MAX_SYM_LEN];
    ir_kind_t kind;
    oprd_t    tar, lhs, rhs;
    op_kind_t op;
    IR_t     *prev, *next;
};

struct IR_list {
    IR_t  *head, *tail;
    oprd_t var;
    u32    size;
};

struct block_t {
    ir_list instrs;
    u32     blk_id;
};

struct IR_fun_t {
    char      str[MAX_SYM_LEN];
    ir_list   instrs;
    ir_fun_t *next;
};

#define ir_iter(IR, IT) for (IR_t * (IT) = (IR); (IT) != NULL; (IT) = (IT)->next)
#define ir_foreach(IR, FUN) ir_iter((IR), __it) FUN(__it);

void ir_append(ir_list *list, IR_t *ir);

void ir_concat(ir_list *front, ir_list *back);

typedef void (*ir_visitor_fun_t)(IR_t *, void *);

#define IR_VISIT(NAME) ir_visitor_fun_t visit_##NAME;

struct IR_visitor {
    char name[MAX_SYM_LEN];
    IR(IR_VISIT)
};

#define IR_EXTEND(NAME) typedef struct IR_t NAME##_t;

IR(IR_EXTEND);

void ir_fun_print(ir_fun_t *fun);

IR_t *ir_alloc(ir_kind_t kind, ...);

oprd_t var_alloc(const char *name);

oprd_t lit_alloc(u32 value);

char *oprd_to_str(oprd_t oprd);

void ast_gen(AST_t *node, ir_list *list);
