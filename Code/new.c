#include "visitor.h"
#include "common.h"
#include "symtab.h"
#include <stdarg.h>

#define RET_TYPE va_list
#define ARG ap
VISITOR_DEF(AST_NODES, new, va_list);

ast_t *ast_alloc(ast_kind_t kind, u32 fst_l, ...) {
#define AST_NODE_ALLOC(NODE)            \
    case NODE:                          \
        ptr = zalloc(sizeof(NODE##_t)); \
        break;

    va_list ap;
    ast_t  *ptr = NULL;
    va_start(ap, fst_l);

    switch (kind) {
        AST_NODES(AST_NODE_ALLOC)
        default: UNREACHABLE;
    }
    ptr->kind  = kind;
    ptr->fst_l = fst_l;
    ptr->next  = NULL;
    visitor_dispatch(visitor_new, ptr, ap);

    va_end(ap);
    return ptr;
}

VISIT(EXPR_INT) {
    node->value = va_arg(ap, i32);
    LOG("%d", node->value);
}

VISIT(EXPR_FLT) {
    node->value = va_arg(ap, f32);
}

VISIT(EXPR_BIN) {
    node->lhs = va_arg(ap, ast_t *);
    node->op  = va_arg(ap, op_kind_t);
    node->rhs = va_arg(ap, ast_t *);
}

VISIT(EXPR_UNR) {
    node->op  = va_arg(ap, op_kind_t);
    node->sub = va_arg(ap, ast_t *);
}

VISIT(EXPR_IDEN) {
    symmov(node->str, va_arg(ap, char *));
}

VISIT(STMT_RET) {
    node->expr = va_arg(ap, ast_t *);
}

VISIT(STMT_WHLE) {
    node->cond = va_arg(ap, ast_t *);
    node->body = va_arg(ap, ast_t *);
}

VISIT(STMT_IFTE) {
    node->cond     = va_arg(ap, ast_t *);
    node->tru_stmt = va_arg(ap, ast_t *);
    node->fls_stmt = va_arg(ap, ast_t *);
}

VISIT(STMT_SCOP) {
    node->decls = va_arg(ap, ast_t *);
    node->stmts = va_arg(ap, ast_t *);
}

VISIT(EXPR_DOT) {
    node->base = va_arg(ap, ast_t *);
    symmov(node->str, va_arg(ap, char *));
}

VISIT(EXPR_ASS) {
    node->lhs = va_arg(ap, ast_t *);
    node->rhs = va_arg(ap, ast_t *);
}

VISIT(CONS_PROG) {
    node->decls = va_arg(ap, ast_t *);
}

VISIT(CONS_FUN) {
    symmov(node->str, va_arg(ap, char *));
    node->params = va_arg(ap, ast_t *);
}

VISIT(DECL_VAR) {
    symmov(node->str, va_arg(ap, char *));
    node->dim = 0;
    LOG("   %s", node->str);
}

VISIT(EXPR_ARR) {
    node->arr = va_arg(ap, ast_t *);
    node->ind = va_arg(ap, ast_t *);
}

VISIT(STMT_EXPR) {
    node->expr = va_arg(ap, ast_t *);
}

VISIT(EXPR_CALL) {
    symmov(node->str, va_arg(ap, char *));
    node->expr = va_arg(ap, ast_t *);
}

VISIT(DECL_TYP) {
}

VISIT(CONS_SPEC) {
    node->kind = va_arg(ap, enum type_kind);
    switch (node->kind) {
        case TYPE_PRIM_INT:
        case TYPE_PRIM_FLT:
            break;
        case TYPE_STRUCT:
            symmov(node->str, va_arg(ap, char *));
            node->fields = va_arg(ap, ast_t *);
            node->is_ref = va_arg(ap, i32);
            break;
        default: UNREACHABLE;
    }
}

VISIT(DECL_FUN) {
}