#include "ast.h"
#include "symtab.h"
#include <stdarg.h>

VISITOR_DEF(new, va_list);

ast_t *ast_alloc(ast_kind_t kind, u32 fst_l, ...) {
#define AST_NODE_ALLOC(NODE)                 \
    case NODE:                               \
        ptr = zalloc(sizeof(NODE##_node_t)); \
        break;

    va_list ap;
    ast_t  *ptr;
    va_start(ap, fst_l);

    switch (kind) {
        AST_NODES(AST_NODE_ALLOC)
        default: assert(0);
    }
    ptr->ast_kind = kind;
    ptr->fst_l    = fst_l;
    ptr->next     = NULL;
    visitor_dispatch(visitor_new, ptr, ap);

    va_end(ap);
    return ptr;
}

static void visit_EXPR_INT(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_INT);
    cnode->value = va_arg(ap, i32);
    LOG("%d", cnode->value);
}

static void visit_EXPR_FLT(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_FLT);
    cnode->value = va_arg(ap, f32);
}

static void visit_EXPR_BIN(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_BIN);
    cnode->lhs = va_arg(ap, ast_t *);
    cnode->op  = va_arg(ap, op_kind_t);
    cnode->rhs = va_arg(ap, ast_t *);
}

static void visit_EXPR_UNR(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_UNR);
    cnode->op  = va_arg(ap, op_kind_t);
    cnode->sub = va_arg(ap, ast_t *);
}

static void visit_EXPR_IDEN(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_IDEN);
    symmov(cnode->str, va_arg(ap, char *));
}

static void visit_STMT_RET(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_RET);
    cnode->expr = va_arg(ap, ast_t *);
}

static void visit_STMT_WHLE(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_WHLE);
    cnode->cond = va_arg(ap, ast_t *);
    cnode->body = va_arg(ap, ast_t *);
}

static void visit_STMT_IFTE(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_IFTE);
    cnode->cond     = va_arg(ap, ast_t *);
    cnode->tru_stmt = va_arg(ap, ast_t *);
    cnode->fls_stmt = va_arg(ap, ast_t *);
}

static void visit_STMT_SCOP(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_SCOP);
    cnode->decls = va_arg(ap, ast_t *);
    cnode->stmts = va_arg(ap, ast_t *);
}

static void visit_EXPR_DOT(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_DOT);
    cnode->base = va_arg(ap, ast_t *);
    symmov(cnode->str, va_arg(ap, char *));
}

static void visit_EXPR_ASS(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_ASS);
    cnode->lhs = va_arg(ap, ast_t *);
    cnode->rhs = va_arg(ap, ast_t *);
}

static void visit_CONS_PROG(ast_t *node, va_list ap) {
    INSTANCE_OF(node, CONS_PROG);
    cnode->decls = va_arg(ap, ast_t *);
}

static void visit_DECL_FUN(ast_t *node, va_list ap) {
    INSTANCE_OF(node, DECL_FUN);
    symmov(cnode->str, va_arg(ap, char *));
    cnode->params = va_arg(ap, ast_t *);
}

static void visit_DECL_VAR(ast_t *node, va_list ap) {
    INSTANCE_OF(node, DECL_VAR);
    symmov(cnode->str, va_arg(ap, char *));
    cnode->dim = va_arg(ap, i32);
    LOG("   %s", cnode->str);
}

static void visit_EXPR_ARR(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_ARR);
    cnode->arr = va_arg(ap, ast_t *);
    cnode->ind = va_arg(ap, ast_t *);
}

static void visit_STMT_EXPR(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_EXPR);
    cnode->expr = va_arg(ap, ast_t *);
}

static void visit_EXPR_CALL(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_CALL);
    symmov(cnode->str, va_arg(ap, char *));
    cnode->expr = va_arg(ap, ast_t *);
}

static void visit_DECL_TYP(ast_t *node, va_list ap) {
    INSTANCE_OF(node, DECL_TYP);
    cnode->typ = va_arg(ap, type_t);
}