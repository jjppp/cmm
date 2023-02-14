#include "../Code/ast.h"

// (1 + 4) + (-2 + 8)
void test_add() {
    extern const struct ast_visitor visitor_eval;
    static ast_t                   *nodes[20];

    nodes[0] = new_ast_node(EXPR_FLT, 0, 1.0);
    nodes[1] = new_ast_node(EXPR_FLT, 0, 4.0);
    nodes[2] = new_ast_node(EXPR_BIN, 0, nodes[0], OP_ADD, nodes[1]);
    nodes[3] = new_ast_node(EXPR_FLT, 0, -2.0);
    nodes[4] = new_ast_node(EXPR_FLT, 0, 8.0);
    nodes[5] = new_ast_node(EXPR_BIN, 0, nodes[3], OP_ADD, nodes[4]);
    nodes[6] = new_ast_node(EXPR_BIN, 0, nodes[2], OP_ADD, nodes[5]);

    f32 result;
    visitor_dispatch(visitor_eval, nodes[6], &result);
    assert(result == 1.0 + 4.0 - 2.0 + 8.0);
}

extern void print(FILE *file, ast_t *node, ...);

void test_print() {
    static ast_t *nodes[20];

    nodes[0] = new_ast_node(EXPR_FLT, 0, 1.0);
    nodes[1] = new_ast_node(EXPR_FLT, 0, 4.0);
    nodes[2] = new_ast_node(EXPR_BIN, 0, nodes[0], OP_ADD, nodes[1]);
    nodes[3] = new_ast_node(EXPR_FLT, 0, -2.0);
    nodes[4] = new_ast_node(EXPR_FLT, 0, 8.0);
    nodes[5] = new_ast_node(EXPR_BIN, 0, nodes[3], OP_ADD, nodes[4]);
    nodes[6] = new_ast_node(EXPR_BIN, 0, nodes[2], OP_ADD, nodes[5]);

    print(stdout, nodes[6]);
}

int main() {
    test_add();
    test_print();
}