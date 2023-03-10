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
    fact_t fact = fact_get(out, node->tar);
    if (fact.kind == FACT_CONST) {
        node->lhs = lit_alloc(fact.val);
    }
}

VISIT(IR_BINARY) {
    fact_t fact = fact_get(out, node->tar);
    if (fact.kind == FACT_CONST) {
        node->kind = IR_ASSIGN;
        node->lhs  = lit_alloc(fact.val);
    } else {
        fact_t lhs_fact = fact_get(out, node->lhs);
        fact_t rhs_fact = fact_get(out, node->rhs);
        if (lhs_fact.kind == FACT_CONST) {
            node->lhs = lit_alloc(lhs_fact.val);
        }
        if (rhs_fact.kind == FACT_CONST) {
            node->rhs = lit_alloc(rhs_fact.val);
        }
    }
}

VISIT(IR_BRANCH) {
    fact_t lhs_fact = fact_get(out, node->lhs);
    fact_t rhs_fact = fact_get(out, node->rhs);
    if (node->lhs.kind == OPRD_VAR && lhs_fact.kind == FACT_CONST) {
        node->lhs = lit_alloc(lhs_fact.val);
    }
    if (node->rhs.kind == OPRD_VAR && rhs_fact.kind == FACT_CONST) {
        node->rhs = lit_alloc(rhs_fact.val);
    }
}

VISIT(IR_RETURN) {
    fact_t fact = fact_get(out, node->lhs);
    if (node->lhs.kind == OPRD_VAR && fact.kind == FACT_CONST) {
        node->lhs = lit_alloc(fact.val);
    }
}

VISIT(IR_ARG) {
    fact_t fact = fact_get(out, node->lhs);
    if (node->lhs.kind == OPRD_VAR && fact.kind == FACT_CONST) {
        node->lhs = lit_alloc(fact.val);
    }
}

VISIT(IR_WRITE) {
    fact_t fact = fact_get(out, node->lhs);
    if (node->lhs.kind == OPRD_VAR && fact.kind == FACT_CONST) {
        node->lhs = lit_alloc(fact.val);
    }
}

VISIT(IR_STORE) {
    fact_t fact = fact_get(out, node->lhs);
    if (node->lhs.kind == OPRD_VAR && fact.kind == FACT_CONST) {
        node->lhs = lit_alloc(fact.val);
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