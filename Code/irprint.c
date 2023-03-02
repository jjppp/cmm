#include "common.h"
#include "ir.h"
#include "visitor.h"
#include <stdio.h>

#define RET_TYPE va_list
#define ARG p_res
VISITOR_DEF(IR, print, RET_TYPE);

static FILE *fout;

static void ir_print_(IR_t *ir) {
    VISITOR_DISPATCH(IR, print, ir, NULL);
}

void ir_print(FILE *file, IR_t *ir) {
    fout = file;
    ir_print_(ir);
}

void ir_fun_print(FILE *file, ir_fun_t *fun) {
    fout = file;
    LIST_ITER(fun, it) {
        fprintf(fout, "FUNCTION %s :\n", it->str);
        LIST_FOREACH(it->instrs.head, ir_print_);
        fprintf(fout, "\n");
    }
}

VISIT(IR_LABEL) {
    fprintf(fout, "LABEL %s :\n", node->str);
}

VISIT(IR_ASSIGN) {
    fprintf(fout, "%s := ", oprd_to_str(node->tar));
    fprintf(fout, "%s\n", oprd_to_str(node->lhs));
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

    fprintf(fout, "%s := ", oprd_to_str(node->tar));
    fprintf(fout, "%s %s ", oprd_to_str(node->lhs), op_str);
    fprintf(fout, "%s\n", oprd_to_str(node->rhs));
}

VISIT(IR_DREF) {
    fprintf(fout, "%s := ", oprd_to_str(node->tar));
    fprintf(fout, "&%s\n", oprd_to_str(node->lhs));
}

VISIT(IR_LOAD) {
    fprintf(fout, "%s := ", oprd_to_str(node->tar));
    fprintf(fout, "*%s\n", oprd_to_str(node->lhs));
}

VISIT(IR_STORE) {
    fprintf(fout, "*%s := ", oprd_to_str(node->tar));
    fprintf(fout, "%s\n", oprd_to_str(node->lhs));
}

VISIT(IR_GOTO) {
    fprintf(fout, "GOTO %s\n", node->jmpto->str);
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
    fprintf(fout, "IF %s %s ", oprd_to_str(node->lhs), op_str);
    fprintf(fout, "%s GOTO %s\n", oprd_to_str(node->rhs), node->jmpto->str);
}

VISIT(IR_RETURN) {
    fprintf(fout, "RETURN %s\n", oprd_to_str(node->lhs));
}

VISIT(IR_DEC) {
    fprintf(fout, "DEC %s ", oprd_to_str(node->tar));
    fprintf(fout, "%s\n", oprd_to_str(node->lhs) + 1);
}

VISIT(IR_ARG) {
    fprintf(fout, "ARG %s\n", oprd_to_str(node->lhs));
}

VISIT(IR_PARAM) {
    fprintf(fout, "PARAM %s\n", oprd_to_str(node->lhs));
}

VISIT(IR_CALL) {
    fprintf(fout, "%s := CALL ", oprd_to_str(node->tar));
    fprintf(fout, "%s\n", node->str);
}

VISIT(IR_READ) {
    fprintf(fout, "READ %s\n", oprd_to_str(node->tar));
}

VISIT(IR_WRITE) {
    fprintf(fout, "WRITE %s\n", oprd_to_str(node->lhs));
}

VISIT_UNDEF(IR_UNARY);
VISIT_UNDEF(IR_NULL);