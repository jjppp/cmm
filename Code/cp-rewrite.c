#include "common.h"
#include "cp.h"
#include "ir.h"

#define RET_TYPE cp_data_t *
#define ARG out
VISITOR_DEF(IR, cp_rewrite, RET_TYPE);

void cp_rewrite(IR_t *ir, cp_data_t *out) {
    ASSERT(out->super.magic == MAGIC, "data magic");
    VISITOR_DISPATCH(IR, cp_rewrite, ir, out);
}

static void rewrite(oprd_t *oprd, fact_t fact) {
    if (fact.kind == FACT_CONST) {
        *oprd = lit_alloc(fact.val);
    }
}

VISIT(IR_ASSIGN) {
    rewrite(&node->lhs, fact_get(out, node->tar));
}

VISIT(IR_BINARY) {
    fact_t fact = fact_get(out, node->tar);
    if (fact.kind == FACT_CONST) {
        node->kind = IR_ASSIGN;
        node->lhs  = lit_alloc(fact.val);
    } else {
        rewrite(&node->lhs, fact_get(out, node->lhs));
        rewrite(&node->rhs, fact_get(out, node->rhs));
    }
}

VISIT(IR_BRANCH) {
    rewrite(&node->lhs, fact_get(out, node->lhs));
    rewrite(&node->rhs, fact_get(out, node->rhs));

    if (node->lhs.kind == OPRD_LIT && node->rhs.kind == OPRD_LIT) {
        i32 lhs_val = node->lhs.val;
        i32 rhs_val = node->rhs.val;
        node->kind  = IR_GOTO;
        IR_t *jmpto = NULL;
        switch (node->op) {
            case OP_LE: {
                jmpto = (lhs_val <= rhs_val) ? node->jmpto : node->next;
                break;
            }
            case OP_LT: {
                jmpto = (lhs_val < rhs_val) ? node->jmpto : node->next;
                break;
            }
            case OP_GE: {
                jmpto = (lhs_val >= rhs_val) ? node->jmpto : node->next;
                break;
            }
            case OP_GT: {
                jmpto = (lhs_val > rhs_val) ? node->jmpto : node->next;
                break;
            }
            case OP_EQ: {
                jmpto = (lhs_val == rhs_val) ? node->jmpto : node->next;
                break;
            }
            case OP_NE: {
                jmpto = (lhs_val != rhs_val) ? node->jmpto : node->next;
                break;
            }
            default: UNREACHABLE;
        }
        node->jmpto = jmpto;
#define DEAD_JMP(EDGE) ((EDGE)->to != jmpto->parent)
        LIST_REMOVE(node->parent->fedge, zfree, DEAD_JMP);
    }
}

VISIT(IR_RETURN) {
    rewrite(&node->lhs, fact_get(out, node->lhs));
}

VISIT(IR_ARG) {
    rewrite(&node->lhs, fact_get(out, node->lhs));
}

VISIT(IR_WRITE) {
    rewrite(&node->lhs, fact_get(out, node->lhs));
}

VISIT(IR_STORE) {
    rewrite(&node->lhs, fact_get(out, node->lhs));
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