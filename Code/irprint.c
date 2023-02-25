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
    char op_ch;
    switch (node->op) {
        case OP_ADD: op_ch = '+'; break;
        case OP_SUB: op_ch = '-'; break;
        case OP_MUL: op_ch = '*'; break;
        case OP_DIV: op_ch = '/'; break;
        case OP_MOD: op_ch = '%'; break;
        default: TODO("irprint OP");
    }

    printf("%s := ", oprd_to_str(node->tar));
    printf("%s %c ", oprd_to_str(node->lhs), op_ch);
    printf("%s\n", oprd_to_str(node->rhs));
}

VISIT(IR_UNARY) {
    TODO("IR_UNARY");
}

VISIT(IR_DREF) {
    TODO("IR_DREF");
}

VISIT(IR_LOAD) {
    TODO("IR_LOAD");
}

VISIT(IR_STORE) {
    TODO("IR_STORE");
}

VISIT(IR_GOTO) {
    TODO("IR_GOTO");
}

VISIT(IR_BRANCH) {
    TODO("IR_BRANCH");
}

VISIT(IR_RETURN) {
    printf("RETURN %s\n", oprd_to_str(node->lhs));
}

VISIT(IR_DEC) {
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
}

VISIT(IR_WRITE) {
}