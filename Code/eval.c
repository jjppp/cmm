#include "ast.h"
#include "common.h"

VISITOR_DEF(eval, f32 *);

static void visit_EXPR_INT(ast_t *node, f32 *p_res) {
    INSTANCE_OF(node, EXPR_INT);
    *p_res = cnode->value;
}

static void visit_EXPR_FLT(ast_t *node, f32 *p_res) {
    INSTANCE_OF(node, EXPR_FLT);
    *p_res = cnode->value;
}

static void visit_EXPR_BIN(ast_t *node, f32 *p_res) {
    INSTANCE_OF(node, EXPR_BIN);
    f32 lhs, rhs;
    visitor_dispatch(visitor_eval, cnode->lhs, &lhs);
    visitor_dispatch(visitor_eval, cnode->rhs, &rhs);
    switch (cnode->op) {
        case OP_ADD: *p_res = lhs + rhs; break;
        case OP_SUB: *p_res = lhs - rhs; break;
        case OP_MUL: *p_res = lhs * rhs; break;
        case OP_DIV: *p_res = lhs / rhs; break;
        default: UNREACHABLE;
    }
}

static void visit_EXPR_UNR(ast_t *node, f32 *p_res) {
    INSTANCE_OF(node, EXPR_UNR);
    f32 sub;
    visitor_dispatch(visitor_eval, cnode->sub, &sub);
    switch (cnode->op) {
        case OP_NEG: *p_res = -sub; break;
        default: UNREACHABLE;
    }
}

static void visit_EXPR_IDEN(ast_t *node, f32 *p_res) {
}

static void visit_STMT_RET(ast_t *node, f32 *p_res) {
}

static void visit_STMT_WHLE(ast_t *node, f32 *p_res) {
}

static void visit_STMT_IFTE(ast_t *node, f32 *p_res) {
}

static void visit_STMT_SCOP(ast_t *node, f32 *p_res) {
}

static void visit_EXPR_DOT(ast_t *node, f32 *p_res) {
}

static void visit_EXPR_ASS(ast_t *node, f32 *p_res) {
}

static void visit_CONS_PROG(ast_t *node, f32 *p_res) {
}

static void visit_DECL_FUN(ast_t *node, f32 *p_res) {
}

static void visit_DECL_VAR(ast_t *node, f32 *p_res) {
}

static void visit_EXPR_ARR(ast_t *node, f32 *p_res) {
}

static void visit_STMT_EXPR(ast_t *node, f32 *p_res) {
}

static void visit_EXPR_CALL(ast_t *node, f32 *p_res) {
}

static void visit_DECL_TYP(ast_t *node, f32 *p_res) {
}

static void visit_CONS_SPEC(ast_t *node, f32 *p_res) {
}