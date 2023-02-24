#pragma once

#include "common.h"
#include "symtab.h"

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
    ast_kind_t kind;
};

#define AST_NODE_EXTEND(NODE) typedef struct NODE##_t NODE##_t;

AST_NODES(AST_NODE_EXTEND);

struct CONS_PROG_t {
    EXTENDS(ast_t);
    ast_t *decls;
};

/* Specifier */
struct CONS_SPEC_t {
    EXTENDS(ast_t);
    enum type_kind kind;
    char           str[MAX_SYM_LEN];
    ast_t         *fields;
    u32            nfield;
    bool           is_ref, done;
};

/* Stmts */
struct STMT_SCOP_t {
    EXTENDS(ast_t);
    ast_t *decls, *stmts;
};

struct STMT_IFTE_t {
    EXTENDS(ast_t);
    ast_t *cond, *tru_stmt, *fls_stmt;
};

struct STMT_WHLE_t {
    EXTENDS(ast_t);
    ast_t *cond, *body;
};

struct STMT_EXPR_t {
    EXTENDS(ast_t);
    ast_t *expr;
};

struct STMT_RET_t {
    EXTENDS(ast_t);
    ast_t *expr;
};

/* Exprs */
struct EXPR_IDEN_t {
    EXTENDS(ast_t);
    syment_t *sym;
    char      str[MAX_SYM_LEN];
};

struct EXPR_CALL_t {
    EXTENDS(ast_t);
    syment_t *fun;
    ast_t    *expr; // nullable
    char      str[MAX_SYM_LEN];
    u32       nexpr;
};

struct EXPR_ASS_t {
    EXTENDS(ast_t);
    ast_t *lhs, *rhs;
};

struct EXPR_ARR_t {
    EXTENDS(ast_t);
    ast_t *arr, *ind; // indices
    u32    nind;
};

struct EXPR_DOT_t {
    EXTENDS(ast_t);
    char   str[MAX_SYM_LEN];
    ast_t *base;
};

struct EXPR_INT_t {
    EXTENDS(ast_t);
    i32 value;
};

struct EXPR_FLT_t {
    EXTENDS(ast_t);
    f32 value;
};

struct EXPR_BIN_t {
    EXTENDS(ast_t);
    ast_t    *lhs;
    ast_t    *rhs;
    op_kind_t op;
};

struct EXPR_UNR_t {
    EXTENDS(ast_t);
    ast_t    *sub;
    op_kind_t op;
};

struct DECL_VAR_t {
    EXTENDS(ast_t);
    syment_t *sym;
    ast_t    *spec;
    ast_t    *expr; // init val
    char      str[MAX_SYM_LEN];
    u32       len[MAX_DIM], dim;
};

struct CONS_FUN_t {
    EXTENDS(ast_t);
    char      str[MAX_SYM_LEN];
    syment_t *sym;
    ast_t    *params;
    ast_t    *spec;
    ast_t    *body;
    u32       nparam;
};

struct DECL_TYP_t {
    EXTENDS(ast_t);
    ast_t *spec;
};

struct DECL_FUN_t {
    EXTENDS(ast_t);
    char      str[MAX_SYM_LEN];
    syment_t *sym;
    ast_t    *params;
    ast_t    *spec;
    u32       nparam;
};

#define INSTANCE_OF(NODE, KIND) INSTANCE_OF_VAR(NODE, KIND, cnode)

#define INSTANCE_OF_VAR(NODE, KIND, CNODE)                                   \
    for (KIND##_t *CNODE = (KIND##_t *) NODE,                                \
                  __once = (KIND##_t){.super = {.super = {.nref = 0}}};      \
         ASSERT((NODE)->kind == KIND, "instanceof %s", AST_NODE_NAMES[KIND]) \
         && !__once.super.super.nref;                                        \
         __once.super.super.nref++)

#define AST_NODE_VISIT(NODE) ast_visitor_fun_t visit_##NODE;
typedef void (*ast_visitor_fun_t)(ast_t *, void *);

struct AST_NODES_visitor {
    char name[MAX_SYM_LEN];
    AST_NODES(AST_NODE_VISIT)
};

ast_t *ast_alloc(ast_kind_t kind, u32 fst_l, ...);

void ast_free(ast_t *node);

type_t ast_check(ast_t *node);

bool ast_lval(ast_t *node);
