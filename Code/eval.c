#include "ast.h"
#include "common.h"

const struct ast_visitor visitor_eval;

static void visit_int(ast_t *node, void *p_res) {
    INSTANCE_OF(node, EXPR_INT);
    *(f32 *) p_res = cnode->value;
}

static void visit_flt(ast_t *node, void *p_res) {
    INSTANCE_OF(node, EXPR_FLT);
    *(f32 *) p_res = cnode->value;
}

static void visit_bin(ast_t *node, void *p_res) {
    INSTANCE_OF(node, EXPR_BIN);
    f32 lhs, rhs;
    visitor_dispatch(visitor_eval, cnode->lhs, &lhs);
    visitor_dispatch(visitor_eval, cnode->rhs, &rhs);
    switch (cnode->op) {
        case OP_ADD: *(f32 *) p_res = lhs + rhs; break;
        case OP_SUB: *(f32 *) p_res = lhs - rhs; break;
        case OP_MUL: *(f32 *) p_res = lhs * rhs; break;
        case OP_DIV: *(f32 *) p_res = lhs / rhs; break;
        default: UNREACHABLE;
    }
}

static void visit_unr(ast_t *node, void *p_res) {
    INSTANCE_OF(node, EXPR_UNR);
    f32 sub;
    visitor_dispatch(visitor_eval, cnode->sub, &sub);
    switch (cnode->op) {
        case OP_NEG: *(f32 *) p_res = -sub; break;
        default: UNREACHABLE;
    }
}

const struct ast_visitor visitor_eval = (struct ast_visitor){
    .name           = "eval",
    .visit_EXPR_INT = visit_int,
    .visit_EXPR_FLT = visit_flt,
    .visit_EXPR_BIN = visit_bin,
    .visit_EXPR_UNR = visit_unr};
