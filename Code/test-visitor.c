#include "ast.h"
#include <assert.h>

// (1 + 4) + (-2 + 8)
void test_add() {
    extern const ast_visitor_t visitor_eval;
    static ast_node_t         *nodes[20];

    nodes[0] = new_ast_node(EXPR_FLT);
    nodes[1] = new_ast_node(EXPR_FLT);
    nodes[2] = new_ast_node(EXPR_BIN);
    nodes[3] = new_ast_node(EXPR_FLT);
    nodes[4] = new_ast_node(EXPR_FLT);
    nodes[5] = new_ast_node(EXPR_BIN);
    nodes[6] = new_ast_node(EXPR_BIN);

    ((EXPR_FLT_node_t *) nodes[0])->value = 1.0;
    ((EXPR_FLT_node_t *) nodes[1])->value = 4.0;
    ((EXPR_FLT_node_t *) nodes[3])->value = -2.0;
    ((EXPR_FLT_node_t *) nodes[4])->value = 8.0;

    ((EXPR_BIN_node_t *) nodes[2])->lhs = nodes[0];
    ((EXPR_BIN_node_t *) nodes[2])->rhs = nodes[1];
    ((EXPR_BIN_node_t *) nodes[5])->lhs = nodes[3];
    ((EXPR_BIN_node_t *) nodes[5])->rhs = nodes[4];
    ((EXPR_BIN_node_t *) nodes[6])->lhs = nodes[2];
    ((EXPR_BIN_node_t *) nodes[6])->rhs = nodes[5];
    f32 result;
    visitor_dispatch(visitor_eval, nodes[6], &result);
    assert(result == 1.0 + 4.0 - 2.0 + 8.0);
}

int main() {
    test_add();
}