#include "ast.h"
#include "common.h"
#include "symtab.h"

VISITOR_DEF(del, va_list);

void ast_free(ast_t *node) {
    if (node == NULL) {
        return;
    }
    ast_free(node->next);
    visitor_dispatch(visitor_del, node, NULL);
    zfree(node);
}

static void visit_EXPR_INT(ast_t *node, va_list ap) {
}

static void visit_EXPR_FLT(ast_t *node, va_list ap) {
}

static void visit_EXPR_BIN(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_BIN);
    ast_free(cnode->lhs);
    ast_free(cnode->rhs);
}

static void visit_EXPR_UNR(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_UNR);
    ast_free(cnode->sub);
}

/* sym does not own the syment_t object
 */
static void visit_EXPR_IDEN(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_IDEN);
    POINTS_FREE(cnode->sym);
}

static void visit_STMT_RET(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_RET);
    ast_free(cnode->expr);
}

static void visit_STMT_WHLE(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_WHLE);
    ast_free(cnode->cond);
    ast_free(cnode->body);
}

static void visit_STMT_IFTE(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_IFTE);
    ast_free(cnode->cond);
    ast_free(cnode->tru_stmt);
    ast_free(cnode->fls_stmt);
}

static void visit_STMT_SCOP(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_SCOP);
    ast_free(cnode->decls);
    ast_free(cnode->stmts);
}

static void visit_EXPR_DOT(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_DOT);
    ast_free(cnode->base);
}

static void visit_EXPR_ASS(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_ASS);
    ast_free(cnode->lhs);
    ast_free(cnode->rhs);
}

static void visit_CONS_PROG(ast_t *node, va_list ap) {
    INSTANCE_OF(node, CONS_PROG);
    ast_free(cnode->decls);
}

static void visit_DECL_FUN(ast_t *node, va_list ap) {
    INSTANCE_OF(node, DECL_FUN);
    ast_free(cnode->params);
    ast_free(cnode->body);
    ast_free(cnode->typ.decls);
}

static void visit_DECL_VAR(ast_t *node, va_list ap) {
    INSTANCE_OF(node, DECL_VAR);
    POINTS_FREE(cnode->sym);
    ast_free(cnode->expr);
    ast_free(cnode->typ.decls);
}

static void visit_EXPR_ARR(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_ARR);
    ast_free(cnode->arr);
    ast_free(cnode->ind);
}

static void visit_STMT_EXPR(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_EXPR);
    ast_free(cnode->expr);
}

static void visit_EXPR_CALL(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_CALL);
    // fun->params also point to shared objects
    if (cnode->fun->super.super.nref == 1) {
        syment_t *fsym = NULL;
        sym_foreach(cnode->fun->params, it) {
            POINTS_FREE(fsym);
            fsym = it;
        }
    }
    POINTS_FREE(cnode->fun);
    ast_free(cnode->expr);
}

static void visit_DECL_TYP(ast_t *node, va_list ap) {
    INSTANCE_OF(node, DECL_TYP);
    ast_free(cnode->typ.decls);
}