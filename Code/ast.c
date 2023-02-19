#include "ast.h"
#include "common.h"
#include "symtab.h"
#include <string.h>

#define STRING_LIST(NODE) STRINGIFY(NODE),

const char *AST_NODE_NAMES[] = {AST_NODES(STRING_LIST) "\0"};
const char *OP_NAMES[]       = {OPS(STRING_LIST) "\0"};

void visitor_dispatch(const struct ast_visitor visitor, ast_t *node, void *p) {
    if (node == NULL) {
        return;
    }
    LOG("%s at %s", visitor.name, AST_NODE_NAMES[node->ast_kind]);
#define AST_NODE_DISPATCH(NODE)                                       \
    case NODE:                                                        \
        ASSERT(visitor.visit_##NODE != NULL,                          \
               "%s has no method %s", visitor.name, STRINGIFY(NODE)); \
        visitor.visit_##NODE(node, p);                                \
        break;

    switch (node->ast_kind) {
        AST_NODES(AST_NODE_DISPATCH)
        default: UNREACHABLE;
    }
}