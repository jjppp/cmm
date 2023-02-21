#pragma once

#include "ast.h"

#define VISITOR_METHOD_ASSIGN(NAME) \
    .visit_##NAME = (void *) visit_##NAME,
#define VISITOR_METHOD_DECLARE(NAME, ARG_TYPE) \
    static void visit_##NAME(NAME##_node_t *node, ARG_TYPE);
#define VISITOR_DEF(NAME, ARG_TYPE)                                  \
    AST_NODES_WITH_ARG(VISITOR_METHOD_DECLARE, ARG_TYPE)             \
    const struct ast_visitor visitor_##NAME = (struct ast_visitor) { \
        .name = STRINGIFY(NAME),                                     \
        AST_NODES(VISITOR_METHOD_ASSIGN)                             \
    }

#define AST_NODE_VISIT(NODE) ast_visitor_fun_t visit_##NODE;
#define VISIT(NODE) static void visit_##NODE(NODE##_node_t *node, RET_TYPE ARG)
#define RETURN(RET_VALUE)       \
    do {                        \
        (*(ARG)) = (RET_VALUE); \
        return;                 \
    } while (0)

typedef void (*ast_visitor_fun_t)(ast_t *, void *);

struct ast_visitor {
    char name[MAX_SYM_LEN];
    AST_NODES(AST_NODE_VISIT)
};

ast_t *ast_alloc(ast_kind_t kind, u32 fst_l, ...);

void ast_free(ast_t *node);

void ast_check(ast_t *node, type_t *typ);

bool ast_lval(ast_t *node);

void visitor_dispatch(const struct ast_visitor visitor, ast_t *node, void *p);