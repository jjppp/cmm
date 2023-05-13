#pragma once

#include "ir.h"

#include <stdarg.h>

#define VISITOR_METHOD_ASSIGN(NAME) .visit_##NAME = (void *) visit_##NAME,
#define VISITOR_METHOD_DECLARE(NAME, ARG_TYPE) static void visit_##NAME(NAME##_t *node, ARG_TYPE);
#define VISITOR_DEF(ACCEPTOR, NAME, ARG_TYPE)                                                     \
    ACCEPTOR##_WITH_ARG(VISITOR_METHOD_DECLARE, ARG_TYPE)                                         \
        const struct ACCEPTOR##_visitor ACCEPTOR##_visitor_##NAME = (struct ACCEPTOR##_visitor) { \
        .name = STRINGIFY(NAME),                                                                  \
        ACCEPTOR(VISITOR_METHOD_ASSIGN)                                                           \
    }

#define VISIT(NODE) static void visit_##NODE(NODE##_t *node, RET_TYPE ARG)
#define VISIT_EMPTY(NODE)                                    \
    static void visit_##NODE(NODE##_t *node, RET_TYPE ARG) { \
    }
#define VISIT_UNDEF(NODE) \
    static void visit_##NODE(NODE##_t *node, RET_TYPE ARG) { UNREACHABLE; }
#define VISIT_TODO(NODE) \
    static void visit_##NODE(NODE##_t *node, RET_TYPE ARG) { TODO(STRINGIFY(NODE)); }
#define RETURN(RET_VALUE)       \
    do {                        \
        (*(ARG)) = (RET_VALUE); \
        return;                 \
    } while (0)

// register visitors
#define VISITORS(F) \
    F(IR)

#define VISITOR_DISPATCH_DEF(ACCEPTOR) \
    void ACCEPTOR##_visitor_dispatch(const struct ACCEPTOR##_visitor visitor, ACCEPTOR##_t *node, void *p);

#define VISITOR_DISPATCH(ACCEPTOR, NAME, NODE, ARG) \
    ACCEPTOR##_visitor_dispatch(ACCEPTOR##_visitor_##NAME, (NODE), (ARG))

VISITORS(VISITOR_DISPATCH_DEF);