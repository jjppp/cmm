#include "ast.h"
#include <assert.h>

const ast_visitor_t visitor_eval;

#define INSTANCE_OF(NODE, KIND, CAST_NODE) \
    assert(NODE->ast_kind == KIND);        \
    KIND##_node_t *CAST_NODE = (KIND##_node_t *) NODE;

static void visit_int(ast_node_t *node, void *p_res) {
    INSTANCE_OF(node, EXPR_INT, cnode);
    *(f32 *) p_res = cnode->value;
}

static void visit_flt(ast_node_t *node, void *p_res) {
    INSTANCE_OF(node, EXPR_FLT, cnode);
    *(f32 *) p_res = cnode->value;
}

static void visit_bin(ast_node_t *node, void *p_res) {
    INSTANCE_OF(node, EXPR_BIN, cnode);
    f32 lhs, rhs;
    visitor_dispatch(visitor_eval, cnode->lhs, &lhs);
    visitor_dispatch(visitor_eval, cnode->rhs, &rhs);
    switch (cnode->op) {
        case OP_ADD: *(f32 *) p_res = lhs + rhs; break;
        case OP_SUB: *(f32 *) p_res = lhs - rhs; break;
        case OP_MUL: *(f32 *) p_res = lhs * rhs; break;
        case OP_DIV: *(f32 *) p_res = lhs / rhs; break;
        default: assert(0);
    }
}

static void visit_unr(ast_node_t *node, void *p_res) {
    INSTANCE_OF(node, EXPR_UNR, cnode);
    f32 sub;
    visitor_dispatch(visitor_eval, cnode->sub, &sub);
    switch (cnode->op) {
        case OP_NEG: *(f32 *) p_res = -sub; break;
        default: assert(0);
    }
}

const ast_visitor_t visitor_eval = (ast_visitor_t){
    .visit_EXPR_INT = visit_int,
    .visit_EXPR_FLT = visit_flt,
    .visit_EXPR_BIN = visit_bin,
    .visit_EXPR_UNR = visit_unr};
