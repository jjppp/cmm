#pragma once

#include "common.h"
#include "symtab.h"
#include "cst.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define VISITOR_METHOD_ASSIGN(NAME) \
    .visit_##NAME = (void *) visit_##NAME,
#define VISITOR_METHOD_DECLARE(NAME, ARG_TYPE) \
    static void visit_##NAME(NAME##_node_t *node, ARG_TYPE);
#define VISITOR_DEF(NAME, ARG_TYPE)                                  \
    AST_NODES_WITH_ARG(VISITOR_METHOD_DECLARE, ARG_TYPE)             \
    const struct ast_visitor visitor_##NAME = (struct ast_visitor) { \
        .name = STRINGIFY(NAME),                                     \
        AST_NODES(VISITOR_METHOD_ASSIGN)                             \
    }

#define ast_iter(NODE, IT) for (ast_t * (IT) = (void *) (NODE); (NODE) != NULL && (IT) != NULL; (IT) = (IT)->next)
#define ast_foreach(NODE, FUN) ast_iter(NODE, __it) FUN(__it);

#define sym_iter(SYM, IT) for (syment_t * (IT) = (void *) (SYM); (SYM) != NULL && (IT) != NULL; (IT) = (IT)->next)
#define sym_foreach(SYM, FUN) sym_iter(SYM, __it) FUN(__it);

typedef struct ast_t ast_t;

typedef enum {
    AST_NODES(LIST)
} ast_kind_t;

typedef enum {
    OPS(LIST)
} op_kind_t;

extern const char *AST_NODE_NAMES[];

struct ast_t {
    EXTENDS(shared);
    ast_t     *next;
    u32        fst_l;
    ast_kind_t ast_kind;
};

#define AST_NODE_EXTEND(NODE) \
    typedef struct NODE##_node_t NODE##_node_t;

AST_NODES(AST_NODE_EXTEND);

struct CONS_PROG_node_t {
    EXTENDS(ast_t);
    ast_t *decls;
};

/* Specifier */
struct CONS_SPEC_node_t {
    EXTENDS(ast_t);
    enum type_kind kind;
    char           str[MAX_SYM_LEN];
    ast_t         *fields;
    bool           is_ref, done;
};

/* Stmts */
struct STMT_SCOP_node_t {
    EXTENDS(ast_t);
    ast_t *decls, *stmts;
};

struct STMT_IFTE_node_t {
    EXTENDS(ast_t);
    ast_t *cond, *tru_stmt, *fls_stmt;
};

struct STMT_WHLE_node_t {
    EXTENDS(ast_t);
    ast_t *cond, *body;
};

struct STMT_EXPR_node_t {
    EXTENDS(ast_t);
    ast_t *expr;
};

struct STMT_RET_node_t {
    EXTENDS(ast_t);
    ast_t *expr;
};

/* Exprs */
struct EXPR_IDEN_node_t {
    EXTENDS(ast_t);
    syment_t *sym;
    char      str[MAX_SYM_LEN];
};

struct EXPR_CALL_node_t {
    EXTENDS(ast_t);
    syment_t *fun;
    ast_t    *expr; // nullable
    char      str[MAX_SYM_LEN];
};

struct EXPR_ASS_node_t {
    EXTENDS(ast_t);
    ast_t *lhs, *rhs;
};

struct EXPR_ARR_node_t {
    EXTENDS(ast_t);
    ast_t *arr, *ind;
};

struct EXPR_DOT_node_t {
    EXTENDS(ast_t);
    char   str[MAX_SYM_LEN];
    ast_t *base;
};

struct EXPR_INT_node_t {
    EXTENDS(ast_t);
    i32 value;
};

struct EXPR_FLT_node_t {
    EXTENDS(ast_t);
    f32 value;
};

struct EXPR_BIN_node_t {
    EXTENDS(ast_t);
    ast_t    *lhs;
    ast_t    *rhs;
    op_kind_t op;
};

struct EXPR_UNR_node_t {
    EXTENDS(ast_t);
    ast_t    *sub;
    op_kind_t op;
};

struct DECL_VAR_node_t {
    EXTENDS(ast_t);
    syment_t *sym;
    ast_t    *spec;
    ast_t    *expr; // init val
    char      str[MAX_SYM_LEN];
    u32       len[MAX_DIM], dim;
};

struct DECL_FUN_node_t {
    EXTENDS(ast_t);
    char      str[MAX_SYM_LEN];
    syment_t *sym;
    ast_t    *params;
    ast_t    *spec;
    ast_t    *body;
};

struct DECL_TYP_node_t {
    EXTENDS(ast_t);
    ast_t *spec;
};

struct CONS_ERR_node_t {
    EXTENDS(ast_t);
};

typedef void (*ast_visitor_fun_t)(ast_t *, void *);

#define AST_NODE_VISIT(NODE) \
    ast_visitor_fun_t visit_##NODE;

struct ast_visitor {
    char name[MAX_SYM_LEN];
    AST_NODES(AST_NODE_VISIT)
};

#define VISIT(NODE) \
    static void visit_##NODE(NODE##_node_t *node, RET_TYPE ARG)

#define INSTANCE_OF(NODE, KIND) INSTANCE_OF_VAR(NODE, KIND, cnode)

#define INSTANCE_OF_VAR(NODE, KIND, CNODE)                                        \
    for (KIND##_node_t *CNODE = (KIND##_node_t *) NODE,                           \
                       __once = (KIND##_node_t){.super = {.super = {.nref = 0}}}; \
         ASSERT((NODE)->ast_kind == KIND, "instanceof %s", AST_NODE_NAMES[KIND])  \
         && !__once.super.super.nref;                                             \
         __once.super.super.nref++)

ast_t *ast_alloc(ast_kind_t kind, u32 fst_l, ...);

void ast_free(ast_t *node);

void ast_check(ast_t *node, type_t *typ);

void visitor_dispatch(const struct ast_visitor visitor, ast_t *node, void *p);