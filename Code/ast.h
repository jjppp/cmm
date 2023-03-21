#pragma once

#include "common.h"
#include "type.h"

#define AST(F)       \
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
    F(DECL_TYP)

#define AST_WITH_ARG(F, ARG) \
    F(CONS_PROG, ARG)        \
    F(CONS_SPEC, ARG)        \
    F(CONS_FUN, ARG)         \
    /* Stmts */              \
    F(STMT_EXPR, ARG)        \
    F(STMT_SCOP, ARG)        \
    F(STMT_IFTE, ARG)        \
    F(STMT_WHLE, ARG)        \
    F(STMT_RET, ARG)         \
    /* Exprs */              \
    F(EXPR_CALL, ARG)        \
    F(EXPR_IDEN, ARG)        \
    F(EXPR_ARR, ARG)         \
    F(EXPR_ASS, ARG)         \
    F(EXPR_DOT, ARG)         \
    F(EXPR_INT, ARG)         \
    F(EXPR_FLT, ARG)         \
    F(EXPR_BIN, ARG)         \
    F(EXPR_UNR, ARG)         \
    /* Constructs */         \
    F(DECL_VAR, ARG)         \
    F(DECL_TYP, ARG)

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

    struct syment_t *sym;
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
    char     str[MAX_SYM_LEN];
    AST_t   *base;
    field_t *field;
    type_t   typ;
};

struct EXPR_INT_t {
    EXTENDS(AST_t);
    i64 value;
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

    struct syment_t *sym;
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
