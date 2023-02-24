#include "visitor.h"
#include "common.h"

#define RET_TYPE f32 *
#define ARG p_res
VISITOR_DEF(AST, eval, RET_TYPE);

f32 ast_eval(AST_t *node) {
    f32 ARG;
    VISITOR_DISPATCH(AST, eval, node, &ARG);
    return ARG;
}

VISIT(EXPR_INT) {
    RETURN(node->value);
}

VISIT(EXPR_FLT) {
    RETURN(node->value);
}

VISIT(EXPR_BIN) {
    f32 lhs = ast_eval(node->lhs);
    f32 rhs = ast_eval(node->rhs);
    switch (node->op) {
        case OP_ADD: RETURN(lhs + rhs); break;
        case OP_SUB: RETURN(lhs - rhs); break;
        case OP_MUL: RETURN(lhs * rhs); break;
        case OP_DIV: RETURN(lhs / rhs); break;
        default: UNREACHABLE;
    }
}

VISIT(EXPR_UNR) {
    f32 sub = ast_eval(node->sub);
    switch (node->op) {
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

VISIT(CONS_FUN) {
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

VISIT(DECL_FUN) {
}