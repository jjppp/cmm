#include "common.h"
#include "ir.h"
#include "visitor.h"
#include <stdio.h>

#define RET_TYPE va_list
#define ARG p_res
VISITOR_DEF(IR, print, RET_TYPE);

void ir_print(IR_t *ir) {
    VISITOR_DISPATCH(IR, print, ir, NULL);
}

void ir_fun_print(ir_fun_t *fun) {
    for (ir_fun_t *it = fun; it != NULL; it = it->next) {
        printf("FUNCTION %s :\n", it->str);
        ir_foreach(it->instrs.head, ir_print);
        printf("\n");
    }
}

VISIT(IR_LABEL) {
    printf("LABEL %s :\n", node->str);
}

VISIT(IR_ASSIGN) {
    printf("%s := ", oprd_to_str(node->tar));
    printf("%s\n", oprd_to_str(node->lhs));
}

VISIT(IR_BINARY) {
    const char *op_str;
    switch (node->op) {
        case OP_ADD: op_str = "+"; break;
        case OP_SUB: op_str = "-"; break;
        case OP_MUL: op_str = "*"; break;
        case OP_DIV: op_str = "/"; break;
        case OP_MOD: op_str = "%"; break;
        default: UNREACHABLE;
    }

    printf("%s := ", oprd_to_str(node->tar));
    printf("%s %s ", oprd_to_str(node->lhs), op_str);
    printf("%s\n", oprd_to_str(node->rhs));
}

VISIT(IR_DREF) {
    printf("%s := ", oprd_to_str(node->tar));
    printf("&%s\n", oprd_to_str(node->lhs));
}

VISIT(IR_LOAD) {
    printf("%s := ", oprd_to_str(node->tar));
    printf("*%s\n", oprd_to_str(node->lhs));
}

VISIT(IR_STORE) {
    printf("*%s := ", oprd_to_str(node->tar));
    printf("%s\n", oprd_to_str(node->lhs));
}

VISIT(IR_GOTO) {
    printf("GOTO %s\n", node->jmpto->str);
}

VISIT(IR_BRANCH) {
    const char *op_str = NULL;
    switch (node->op) {
        case OP_EQ: op_str = "=="; break;
        case OP_LE: op_str = "<="; break;
        case OP_NE: op_str = "!="; break;
        case OP_LT: op_str = "<"; break;
        case OP_GE: op_str = ">="; break;
        case OP_GT: op_str = ">"; break;
        default: UNREACHABLE;
    }
    printf("IF %s %s ", oprd_to_str(node->lhs), op_str);
    printf("%s GOTO %s\n", oprd_to_str(node->rhs), node->jmpto->str);
}

VISIT(IR_RETURN) {
    printf("RETURN %s\n", oprd_to_str(node->lhs));
}

VISIT(IR_DEC) {
    printf("DEC %s ", oprd_to_str(node->tar));
    printf("%s\n", oprd_to_str(node->lhs));
}

VISIT(IR_ARG) {
    printf("ARG %s\n", oprd_to_str(node->lhs));
}

VISIT(IR_PARAM) {
    printf("PARAM %s\n", oprd_to_str(node->lhs));
}

VISIT(IR_CALL) {
    printf("%s := CALL ", oprd_to_str(node->tar));
    printf("%s\n", node->str);
}

VISIT(IR_READ) {
    printf("READ %s\n", oprd_to_str(node->tar));
}

VISIT(IR_WRITE) {
    printf("WRITE %s\n", oprd_to_str(node->lhs));
}

VISIT_UNDEF(IR_UNARY);