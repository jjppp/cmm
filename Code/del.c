#include "ast.h"
#include "common.h"
#include "symtab.h"
#include <stdarg.h>

#define RET_TYPE va_list
#define ARG ap
VISITOR_DEF(del, va_list);

void ast_free(ast_t *node) {
    if (node == NULL) {
        return;
    }
    ast_free(node->next);
    visitor_dispatch(visitor_del, node, NULL);
    zfree(node);
}

VISIT(EXPR_INT) {
}

VISIT(EXPR_FLT) {
}

VISIT(EXPR_BIN) {
    INSTANCE_OF(node, EXPR_BIN);
    ast_free(cnode->lhs);
    ast_free(cnode->rhs);
}

VISIT(EXPR_UNR) {
    INSTANCE_OF(node, EXPR_UNR);
    ast_free(cnode->sub);
}

/* sym does not own the syment_t object
 */
VISIT(EXPR_IDEN) {
}

VISIT(STMT_RET) {
    INSTANCE_OF(node, STMT_RET);
    ast_free(cnode->expr);
}

VISIT(STMT_WHLE) {
    INSTANCE_OF(node, STMT_WHLE);
    ast_free(cnode->cond);
    ast_free(cnode->body);
}

VISIT(STMT_IFTE) {
    INSTANCE_OF(node, STMT_IFTE);
    ast_free(cnode->cond);
    ast_free(cnode->tru_stmt);
    ast_free(cnode->fls_stmt);
}

VISIT(STMT_SCOP) {
    INSTANCE_OF(node, STMT_SCOP);
    ast_free(cnode->decls);
    ast_free(cnode->stmts);
}

VISIT(EXPR_DOT) {
    INSTANCE_OF(node, EXPR_DOT);
    ast_free(cnode->base);
}

VISIT(EXPR_ASS) {
    INSTANCE_OF(node, EXPR_ASS);
    ast_free(cnode->lhs);
    ast_free(cnode->rhs);
}

VISIT(CONS_PROG) {
    INSTANCE_OF(node, CONS_PROG);
    ast_free(cnode->decls);
}

VISIT(DECL_FUN) {
    INSTANCE_OF(node, DECL_FUN);
    ast_free(cnode->params);
    ast_free(cnode->body);
    POINTS_FREE(cnode->spec, ast_free);
}

VISIT(DECL_VAR) {
    INSTANCE_OF(node, DECL_VAR);
    ast_free(cnode->expr);
    POINTS_FREE(cnode->spec, ast_free);
}

VISIT(EXPR_ARR) {
    INSTANCE_OF(node, EXPR_ARR);
    ast_free(cnode->arr);
    ast_free(cnode->ind);
}

VISIT(STMT_EXPR) {
    INSTANCE_OF(node, STMT_EXPR);
    ast_free(cnode->expr);
}

VISIT(EXPR_CALL) {
    INSTANCE_OF(node, EXPR_CALL);
    ast_free(cnode->expr);
}

VISIT(DECL_TYP) {
    INSTANCE_OF(node, DECL_TYP);
    POINTS_FREE(cnode->spec, ast_free);
}

VISIT(CONS_SPEC) {
    INSTANCE_OF(node, CONS_SPEC);
    ast_free(cnode->fields);
}