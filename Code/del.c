#include "ast.h"

VISITOR_DEF(del, va_list);

void del_ast_node(ast_t *node) {
    if (node == NULL) {
        return;
    }
    del_ast_node(node->next);
    visitor_dispatch(visitor_del, node, NULL);
    zfree(node);
}

static void visit_EXPR_INT(ast_t *node, va_list ap) {
}

static void visit_EXPR_FLT(ast_t *node, va_list ap) {
}

static void visit_EXPR_BIN(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_BIN);
    del_ast_node(cnode->lhs);
    del_ast_node(cnode->rhs);
}

static void visit_EXPR_UNR(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_UNR);
    del_ast_node(cnode->sub);
}

/* sym does not own the syment_t object
 */
static void visit_EXPR_IDEN(ast_t *node, va_list ap) {
}

static void visit_STMT_RET(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_RET);
    del_ast_node(cnode->expr);
}

static void visit_STMT_WHLE(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_WHLE);
    del_ast_node(cnode->cond);
    del_ast_node(cnode->body);
}

static void visit_STMT_IFTE(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_IFTE);
    del_ast_node(cnode->cond);
    del_ast_node(cnode->tru_stmt);
    del_ast_node(cnode->fls_stmt);
}

static void visit_STMT_SCOP(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_SCOP);
    del_ast_node(cnode->decls);
    del_ast_node(cnode->stmts);
}

static void visit_EXPR_DOT(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_DOT);
    del_ast_node(cnode->base);
    del_ast_node(cnode->field);
}

static void visit_EXPR_ASS(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_ASS);
    del_ast_node(cnode->lhs);
    del_ast_node(cnode->rhs);
}

static void visit_CONS_PROG(ast_t *node, va_list ap) {
    INSTANCE_OF(node, CONS_PROG);
    del_ast_node(cnode->decls);
}

static void visit_DECL_FUN(ast_t *node, va_list ap) {
    INSTANCE_OF(node, DECL_FUN);
    del_ast_node(cnode->params);
    del_ast_node(cnode->body);
}

static void visit_DECL_VAR(ast_t *node, va_list ap) {
}

static void visit_EXPR_ARR(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_ARR);
    del_ast_node(cnode->arr);
    del_ast_node(cnode->ind);
}

static void visit_STMT_EXPR(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_EXPR);
    del_ast_node(cnode->expr);
}

static void visit_EXPR_CALL(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_CALL);
    del_ast_node(cnode->expr);
}