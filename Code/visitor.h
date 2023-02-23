#pragma once

#include "ast.h"

#define VISITOR_METHOD_ASSIGN(NAME) \
    .visit_##NAME = (void *) visit_##NAME,
#define VISITOR_METHOD_DECLARE(NAME, ARG_TYPE) \
    static void visit_##NAME(NAME##_t *node, ARG_TYPE);
#define VISITOR_DEF(ACCEPTOR, NAME, ARG_TYPE)                                          \
    ACCEPTOR##_WITH_ARG(VISITOR_METHOD_DECLARE, ARG_TYPE)                              \
        const struct ACCEPTOR##_visitor visitor_##NAME = (struct ACCEPTOR##_visitor) { \
        .name = STRINGIFY(NAME),                                                       \
        ACCEPTOR(VISITOR_METHOD_ASSIGN)                                                \
    }

#define VISIT(NODE) static void visit_##NODE(NODE##_t *node, RET_TYPE ARG)
#define RETURN(RET_VALUE)       \
    do {                        \
        (*(ARG)) = (RET_VALUE); \
        return;                 \
    } while (0)

void visitor_dispatch(const struct AST_NODES_visitor visitor, ast_t *node, void *p);