#include "cp.h"
#include "ir.h"

#define RET_TYPE cp_data_t *
#define ARG out
VISITOR_DEF(IR, cp_rewrite, RET_TYPE);

void cp_rewrite(IR_t *ir, cp_data_t *out) {
    ASSERT(out->super.magic == MAGIC, "data magic");
    VISITOR_DISPATCH(IR, cp_rewrite, ir, out);
}

VISIT(IR_ASSIGN) {
    if (FACT_TAR(tar).kind == FACT_CONST) {
        node->lhs = lit_alloc(FACT_TAR(tar).val);
    }
}

VISIT(IR_BINARY) {
    if (FACT_TAR(tar).kind == FACT_CONST) {
        node->kind = IR_ASSIGN;
        node->lhs  = lit_alloc(FACT_TAR(tar).val);
    }
}

VISIT(IR_BRANCH) {
    if (node->lhs.kind == OPRD_VAR && IS_CONST(lhs)) {
        node->lhs = lit_alloc(FACT_TAR(lhs).val);
    }
    if (node->rhs.kind == OPRD_VAR && IS_CONST(rhs)) {
        node->rhs = lit_alloc(FACT_TAR(rhs).val);
    }
}

VISIT(IR_RETURN) {
    if (node->lhs.kind == OPRD_VAR && IS_CONST(lhs)) {
        node->lhs = lit_alloc(FACT_TAR(lhs).val);
    }
}

VISIT(IR_ARG) {
    if (node->lhs.kind == OPRD_VAR && IS_CONST(lhs)) {
        node->lhs = lit_alloc(FACT_TAR(lhs).val);
    }
}

VISIT(IR_WRITE) {
    if (node->lhs.kind == OPRD_VAR && IS_CONST(lhs)) {
        node->lhs = lit_alloc(FACT_TAR(lhs).val);
    }
}

VISIT(IR_STORE) {
    if (node->lhs.kind == OPRD_VAR && IS_CONST(lhs)) {
        node->lhs = lit_alloc(FACT_TAR(lhs).val);
    }
}

VISIT_UNDEF(IR_NULL);

VISIT_EMPTY(IR_DREF);
VISIT_EMPTY(IR_LOAD);
VISIT_EMPTY(IR_CALL);
VISIT_EMPTY(IR_READ);
VISIT_EMPTY(IR_DEC);
VISIT_EMPTY(IR_PARAM);
VISIT_EMPTY(IR_GOTO);
VISIT_EMPTY(IR_LABEL);