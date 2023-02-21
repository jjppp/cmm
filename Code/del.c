#include "visitor.h"
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
    ast_free(node->lhs);
    ast_free(node->rhs);
}

VISIT(EXPR_UNR) {
    ast_free(node->sub);
}

/* sym does not own the syment_t object
 */
VISIT(EXPR_IDEN) {
}

VISIT(STMT_RET) {
    ast_free(node->expr);
}

VISIT(STMT_WHLE) {
    ast_free(node->cond);
    ast_free(node->body);
}

VISIT(STMT_IFTE) {
    ast_free(node->cond);
    ast_free(node->tru_stmt);
    ast_free(node->fls_stmt);
}

VISIT(STMT_SCOP) {
    ast_free(node->decls);
    ast_free(node->stmts);
}

VISIT(EXPR_DOT) {
    ast_free(node->base);
}

VISIT(EXPR_ASS) {
    ast_free(node->lhs);
    ast_free(node->rhs);
}

VISIT(CONS_PROG) {
    ast_free(node->decls);
}

VISIT(DECL_FUN) {
    ast_free(node->params);
    ast_free(node->body);
    POINTS_FREE(node->spec, ast_free);
}

VISIT(DECL_VAR) {
    ast_free(node->expr);
    POINTS_FREE(node->spec, ast_free);
}

VISIT(EXPR_ARR) {
    ast_free(node->arr);
    ast_free(node->ind);
}

VISIT(STMT_EXPR) {
    ast_free(node->expr);
}

VISIT(EXPR_CALL) {
    ast_free(node->expr);
}

VISIT(DECL_TYP) {
    POINTS_FREE(node->spec, ast_free);
}

VISIT(CONS_SPEC) {
    ast_free(node->fields);
}