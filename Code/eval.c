#include "ast.h"
#include "common.h"

#define RET_TYPE f32 *
#define ARG p_res
VISITOR_DEF(eval, f32 *);

VISIT(EXPR_INT) {
    INSTANCE_OF(node, EXPR_INT);
    *p_res = cnode->value;
}

VISIT(EXPR_FLT) {
    INSTANCE_OF(node, EXPR_FLT);
    *p_res = cnode->value;
}

VISIT(EXPR_BIN) {
    INSTANCE_OF(node, EXPR_BIN);
    f32 lhs, rhs;
    visitor_dispatch(visitor_eval, cnode->lhs, &lhs);
    visitor_dispatch(visitor_eval, cnode->rhs, &rhs);
    switch (cnode->op) {
        case OP_ADD: *p_res = lhs + rhs; break;
        case OP_SUB: *p_res = lhs - rhs; break;
        case OP_MUL: *p_res = lhs * rhs; break;
        case OP_DIV: *p_res = lhs / rhs; break;
        default: UNREACHABLE;
    }
}

VISIT(EXPR_UNR) {
    INSTANCE_OF(node, EXPR_UNR);
    f32 sub;
    visitor_dispatch(visitor_eval, cnode->sub, &sub);
    switch (cnode->op) {
        case OP_NEG: *p_res = -sub; break;
        default: UNREACHABLE;
    }
}

VISIT(EXPR_IDEN) {
}

VISIT(STMT_RET) {
}

VISIT(STMT_WHLE) {
}

VISIT(STMT_IFTE) {
}

VISIT(STMT_SCOP) {
}

VISIT(EXPR_DOT) {
}

VISIT(EXPR_ASS) {
}

VISIT(CONS_PROG) {
}

VISIT(DECL_FUN) {
}

VISIT(DECL_VAR) {
}

VISIT(EXPR_ARR) {
}

VISIT(STMT_EXPR) {
}

VISIT(EXPR_CALL) {
}

VISIT(DECL_TYP) {
}

VISIT(CONS_SPEC) {
}