#include "ir.h"
#include "visitor.h"

#define RET_TYPE va_list
#define ARG p_res
VISITOR_DEF(IR, exec, RET_TYPE);

VISIT(IR_LABEL) {
}

VISIT(IR_ASSIGN) {
}

VISIT(IR_BINARY) {
}

VISIT(IR_UNARY) {
}

VISIT(IR_DREF) {
}

VISIT(IR_LOAD) {
}

VISIT(IR_STORE) {
}

VISIT(IR_GOTO) {
}

VISIT(IR_BRANCH) {
}

VISIT(IR_RETURN) {
}

VISIT(IR_DEC) {
}

VISIT(IR_ARG) {
}

VISIT(IR_PARAM) {
}

VISIT(IR_CALL) {
}

VISIT(IR_READ) {
}

VISIT(IR_WRITE) {
}

VISIT(IR_NULL) {
}