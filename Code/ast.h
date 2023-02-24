#pragma once

#include "common.h"
#include "symtab.h"

#define ast_iter(NODE, IT) for (AST_t * (IT) = (void *) (NODE); (NODE) != NULL && (IT) != NULL; (IT) = (IT)->next)
#define ast_foreach(NODE, FUN) ast_iter(NODE, __it) FUN(__it);

#define sym_iter(SYM, IT) for (syment_t * (IT) = (void *) (SYM); (SYM) != NULL && (IT) != NULL; (IT) = (IT)->next)
#define sym_foreach(SYM, FUN) sym_iter(SYM, __it) FUN(__it);

typedef struct AST_t AST_t;

typedef enum {
    AST(LIST)
} ast_kind_t;

typedef enum {
    OPS(LIST)
} op_kind_t;

extern const char *AST_NAMES[];

struct AST_t {
    EXTENDS(shared);
    AST_t     *next;
    u32        fst_l;
    ast_kind_t kind;
};

#define AST_EXTEND(NODE) typedef struct NODE##_t NODE##_t;

AST(AST_EXTEND);

struct CONS_PROG_t {
    EXTENDS(AST_t);
    AST_t *decls;
};

/* Specifier */
struct CONS_SPEC_t {
    EXTENDS(AST_t);
    enum type_kind kind;
    char           str[MAX_SYM_LEN];
    AST_t         *fields;
    u32            nfield;
    bool           is_ref, done;
};

/* Stmts */
struct STMT_SCOP_t {
    EXTENDS(AST_t);
    AST_t *decls, *stmts;
};

struct STMT_IFTE_t {
    EXTENDS(AST_t);
    AST_t *cond, *tru_stmt, *fls_stmt;
};

struct STMT_WHLE_t {
    EXTENDS(AST_t);
    AST_t *cond, *body;
};

struct STMT_EXPR_t {
    EXTENDS(AST_t);
    AST_t *expr;
};

struct STMT_RET_t {
    EXTENDS(AST_t);
    AST_t *expr;
};

/* Exprs */
struct EXPR_IDEN_t {
    EXTENDS(AST_t);
    char str[MAX_SYM_LEN];
};

struct EXPR_CALL_t {
    EXTENDS(AST_t);
    AST_t *expr; // nullable
    char   str[MAX_SYM_LEN];
    u32    nexpr;
};

struct EXPR_ASS_t {
    EXTENDS(AST_t);
    AST_t *lhs, *rhs;
};

struct EXPR_ARR_t {
    EXTENDS(AST_t);
    AST_t *arr, *ind; // indices
    u32    nind;
};

struct EXPR_DOT_t {
    EXTENDS(AST_t);
    char   str[MAX_SYM_LEN];
    AST_t *base;
};

struct EXPR_INT_t {
    EXTENDS(AST_t);
    i32 value;
};

struct EXPR_FLT_t {
    EXTENDS(AST_t);
    f32 value;
};

struct EXPR_BIN_t {
    EXTENDS(AST_t);
    AST_t    *lhs;
    AST_t    *rhs;
    op_kind_t op;
};

struct EXPR_UNR_t {
    EXTENDS(AST_t);
    AST_t    *sub;
    op_kind_t op;
};

struct DECL_VAR_t {
    EXTENDS(AST_t);
    AST_t *spec;
    AST_t *expr; // init val
    char   str[MAX_SYM_LEN];
    u32    len[MAX_DIM], dim;
};

struct CONS_FUN_t {
    EXTENDS(AST_t);
    char   str[MAX_SYM_LEN];
    AST_t *params;
    AST_t *spec;
    AST_t *body;
    u32    nparam;
};

struct DECL_TYP_t {
    EXTENDS(AST_t);
    AST_t *spec;
};

struct DECL_FUN_t {
    EXTENDS(AST_t);
    char   str[MAX_SYM_LEN];
    AST_t *params;
    AST_t *spec;
    u32    nparam;
};

#define INSTANCE_OF(NODE, KIND) INSTANCE_OF_VAR(NODE, KIND, cnode)

#define INSTANCE_OF_VAR(NODE, KIND, CNODE)                              \
    for (KIND##_t *CNODE = (KIND##_t *) NODE,                           \
                  __once = (KIND##_t){.super = {.super = {.nref = 0}}}; \
         ASSERT((NODE)->kind == KIND, "instanceof %s", AST_NAMES[KIND]) \
         && !__once.super.super.nref;                                   \
         __once.super.super.nref++)

#define AST_VISIT(NODE) ast_visitor_fun_t visit_##NODE;
typedef void (*ast_visitor_fun_t)(AST_t *, void *);

struct AST_visitor {
    char name[MAX_SYM_LEN];
    AST(AST_VISIT)
};

AST_t *ast_alloc(ast_kind_t kind, u32 fst_l, ...);

void ast_free(AST_t *node);

type_t ast_check(AST_t *node);

bool ast_lval(AST_t *node);
