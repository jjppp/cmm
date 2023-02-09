#include "ast.h"
#include <assert.h>
#include <strings.h>

void visitor_dispatch(const ast_visitor_t visitor, ast_node_t *node, void *p) {
#define AST_NODE_DISPATCH(NODE) \
    case NODE: visitor.visit_##NODE(node, p); break;

    switch (node->ast_kind) {
        AST_NODES(AST_NODE_DISPATCH)
        default: assert(0);
    }
}

ast_node_t *new_ast_node(ast_kind_t kind) {
#define AST_NODE_ALLOC(NODE)                 \
    case NODE:                               \
        ptr = zalloc(sizeof(NODE##_node_t)); \
        break;

    ast_node_t *ptr;
    switch (kind) {
        AST_NODES(AST_NODE_ALLOC)
        default: assert(0);
    }
    ptr->ast_kind = kind;
    return ptr;
}
