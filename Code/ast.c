#include "ast.h"
#include "visitor.h"
#include "common.h"
#include "symtab.h"
#include <string.h>

#define STRING_LIST(NODE) STRINGIFY(NODE),

const char *AST_NAMES[] = {AST(STRING_LIST) "\0"};
const char *OP_NAMES[]  = {OPS(STRING_LIST) "\0"};

void AST_visitor_dispatch(const struct AST_visitor visitor, AST_t *node, void *p) {
    if (node == NULL) {
        return;
    }
    LOG("%s at %s", visitor.name, AST_NAMES[node->kind]);
#define AST_DISPATCH(NODE)                                            \
    case NODE:                                                        \
        ASSERT(visitor.visit_##NODE != NULL,                          \
               "%s has no method %s", visitor.name, STRINGIFY(NODE)); \
        visitor.visit_##NODE(node, p);                                \
        break;

    switch (node->kind) {
        AST(AST_DISPATCH)
        default: UNREACHABLE;
    }
}