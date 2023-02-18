#include "ast.h"
#include "common.h"
#include "symtab.h"
#include <stdarg.h>

#define RET_TYPE va_list
#define ARG ap
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

VISIT(EXPR_INT) {
    INSTANCE_OF(node, EXPR_INT);
    cnode->value = va_arg(ap, i32);
    LOG("%d", cnode->value);
}

VISIT(EXPR_FLT) {
    INSTANCE_OF(node, EXPR_FLT);
    cnode->value = va_arg(ap, f32);
}

VISIT(EXPR_BIN) {
    INSTANCE_OF(node, EXPR_BIN);
    cnode->lhs = va_arg(ap, ast_t *);
    cnode->op  = va_arg(ap, op_kind_t);
    cnode->rhs = va_arg(ap, ast_t *);
}

VISIT(EXPR_UNR) {
    INSTANCE_OF(node, EXPR_UNR);
    cnode->op  = va_arg(ap, op_kind_t);
    cnode->sub = va_arg(ap, ast_t *);
}

VISIT(EXPR_IDEN) {
    INSTANCE_OF(node, EXPR_IDEN);
    symmov(cnode->str, va_arg(ap, char *));
}

VISIT(STMT_RET) {
    INSTANCE_OF(node, STMT_RET);
    cnode->expr = va_arg(ap, ast_t *);
}

VISIT(STMT_WHLE) {
    INSTANCE_OF(node, STMT_WHLE);
    cnode->cond = va_arg(ap, ast_t *);
    cnode->body = va_arg(ap, ast_t *);
}

VISIT(STMT_IFTE) {
    INSTANCE_OF(node, STMT_IFTE);
    cnode->cond     = va_arg(ap, ast_t *);
    cnode->tru_stmt = va_arg(ap, ast_t *);
    cnode->fls_stmt = va_arg(ap, ast_t *);
}

VISIT(STMT_SCOP) {
    INSTANCE_OF(node, STMT_SCOP);
    cnode->decls = va_arg(ap, ast_t *);
    cnode->stmts = va_arg(ap, ast_t *);
}

VISIT(EXPR_DOT) {
    INSTANCE_OF(node, EXPR_DOT);
    cnode->base = va_arg(ap, ast_t *);
    symmov(cnode->str, va_arg(ap, char *));
}

VISIT(EXPR_ASS) {
    INSTANCE_OF(node, EXPR_ASS);
    cnode->lhs = va_arg(ap, ast_t *);
    cnode->rhs = va_arg(ap, ast_t *);
}

VISIT(CONS_PROG) {
    INSTANCE_OF(node, CONS_PROG);
    cnode->decls = va_arg(ap, ast_t *);
}

VISIT(DECL_FUN) {
    INSTANCE_OF(node, DECL_FUN);
    symmov(cnode->str, va_arg(ap, char *));
    cnode->params = va_arg(ap, ast_t *);
}

VISIT(DECL_VAR) {
    INSTANCE_OF(node, DECL_VAR);
    symmov(cnode->str, va_arg(ap, char *));
    cnode->dim = 0;
    LOG("   %s", cnode->str);
}

VISIT(EXPR_ARR) {
    INSTANCE_OF(node, EXPR_ARR);
    cnode->arr = va_arg(ap, ast_t *);
    cnode->ind = va_arg(ap, ast_t *);
}

VISIT(STMT_EXPR) {
    INSTANCE_OF(node, STMT_EXPR);
    cnode->expr = va_arg(ap, ast_t *);
}

VISIT(EXPR_CALL) {
    INSTANCE_OF(node, EXPR_CALL);
    symmov(cnode->str, va_arg(ap, char *));
    cnode->expr = va_arg(ap, ast_t *);
}

VISIT(DECL_TYP) {
    INSTANCE_OF(node, DECL_TYP);
    POINTS_TO(cnode->spec, va_arg(ap, ast_t *));
}

VISIT(CONS_SPEC) {
    INSTANCE_OF(node, CONS_SPEC);
    cnode->kind = va_arg(ap, enum type_kind);
    switch (cnode->kind) {
        case TYPE_PRIM_INT:
        case TYPE_PRIM_FLT:
            break;
        case TYPE_STRUCT:
            symmov(cnode->str, va_arg(ap, char *));
            cnode->fields = va_arg(ap, ast_t *);
            cnode->is_ref = va_arg(ap, int);
            break;
        case TYPE_ARRAY:
            TODO("TYPE_ARRAY");
        default: UNREACHABLE;
    }
}