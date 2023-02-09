#ifndef __AST_H__
#define __AST_H__

#include "common.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_CHILD 512

typedef struct ast_node_t ast_node_t;

#define AST_KIND_ENUM(NODE) \
    NODE,

#define AST_NODE_NAME(NODE) \
    STRINGIFY(NODE),

typedef enum {
    AST_NODES(AST_KIND_ENUM)
} ast_kind_t;

static const char *AST_NODE_NAMES[] = {
    AST_NODES(AST_NODE_NAME) "\0"};

struct ast_node_t {
    u32         fst_l;
    ast_kind_t  ast_kind;
    ast_node_t *chld[MAX_CHILD];
};

#define AST_NODE_EXTEND(NODE) \
    typedef struct NODE##_node_t NODE##_node_t;

AST_NODES(AST_NODE_EXTEND);

struct PROGRAM_node_t {
    EXTENDS(ast_node_t);
};

/* Types */
typedef struct type_t {
    enum {
        TYPE_PRIM_INT,
        TYPE_PRIM_FLT,
        TYPE_STRUCT
    } spec_type;
    // TODO: list of var decls
    DECL_VAR_node_t *var_decls[MAX_CHILD];
} type_t;

/* Stmts */
struct STMT_SCOP_node_t {
    EXTENDS(ast_node_t);
    // TODO: list of var decls
    // TODO: list of stmts
    DECL_VAR_node_t *var_decls[MAX_CHILD];
    ast_node_t      *stmts[MAX_CHILD];
};

struct STMT_IFTE_node_t {
    EXTENDS(ast_node_t);
    ast_node_t *cond;
    ast_node_t *tru_stmt;
    ast_node_t *fls_stmt; // nullable
};

struct STMT_WHLE_node_t {
    EXTENDS(ast_node_t);
    ast_node_t *cond;
    ast_node_t *body;
};

struct STMT_RET_node_t {
    EXTENDS(ast_node_t);
    ast_node_t *expr;
};

struct STMT_DOT_node_t {
    EXTENDS(ast_node_t);
    ast_node_t *base;
    ast_node_t *field;
};

struct STMT_ASS_node_t {
    EXTENDS(ast_node_t);
    ast_node_t *lhs;
    ast_node_t *rhs;
};

/* Exprs */
struct EXPR_IDEN_node_t {
    EXTENDS(ast_node_t);
    // TODO: use handle to represent symbols
};

struct EXPR_INT_node_t {
    EXTENDS(ast_node_t);
    i32 value;
};

struct EXPR_FLT_node_t {
    EXTENDS(ast_node_t);
    f32 value;
};

struct EXPR_BIN_node_t {
    EXTENDS(ast_node_t);
    ast_node_t *lhs;
    ast_node_t *rhs;
    enum {
        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OP_MOD,
        OP_AND,
        OP_OR
    } op;
};

struct EXPR_UNR_node_t {
    EXTENDS(ast_node_t);
    ast_node_t *sub;
    enum {
        OP_NEG,
        OP_NOT,
    } op;
};

struct DECL_VAR_node_t {
    EXTENDS(ast_node_t);
    type_t type;
    // TODO: use handle to represent symbols
};

struct DECL_FUN_node_t {
    EXTENDS(ast_node_t);
    type_t type;
    // TODO: use handle to represent symbols
    // TODO: list of parameters
    DECL_VAR_node_t *params[MAX_CHILD];
    ast_node_t      *body;
};

typedef void (*ast_visitor_fun_t)(ast_node_t *, void *);

#define AST_NODE_VISIT(NODE) \
    ast_visitor_fun_t visit_##NODE;

typedef struct ast_visitor_t ast_visitor_t;

struct ast_visitor_t {
    AST_NODES(AST_NODE_VISIT)
};

void visitor_dispatch(const ast_visitor_t visitor, ast_node_t *node, void *p);

ast_node_t *new_ast_node(ast_kind_t kind);

#endif