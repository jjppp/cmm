#include "common.h"
#include "cfg.h"
#include "visitor.h"
#include "ir.h"

#define RET_TYPE va_list
#define ARG out
VISITOR_DEF(IR, strength, RET_TYPE);

static void strength_reduction(IR_t *ir) {
    VISITOR_DISPATCH(IR, strength, ir, NULL);
}

void do_strength(cfg_t *cfg) {
    LIST_ITER(cfg->blocks, blk) {
        LIST_ITER(blk->instrs.head, ir) {
            strength_reduction(ir);
        }
        ir_remove_mark(&blk->instrs);
    }
}

VISIT(IR_BINARY) {
#define IS_LIT(OPRD, LIT) (((OPRD).kind == OPRD_LIT) && ((OPRD).val == LIT))
    oprd_t lhs = node->lhs;
    oprd_t rhs = node->rhs;

    switch (node->op) {
        case OP_ADD: {
            if (IS_LIT(lhs, 0)) {
                node->kind = IR_ASSIGN;
                node->lhs  = rhs;
            } else if (IS_LIT(rhs, 0)) {
                node->kind = IR_ASSIGN;
            }
            break;
        }
        case OP_MUL: {
            if (IS_LIT(lhs, 1)) {
                node->kind = IR_ASSIGN;
                node->lhs  = rhs;
            } else if (IS_LIT(rhs, 1)) {
                node->kind = IR_ASSIGN;
            } else if (IS_LIT(lhs, 2)) {
                node->op  = OP_ADD;
                node->lhs = rhs;
            } else if (IS_LIT(rhs, 2)) {
                node->op  = OP_ADD;
                node->rhs = lhs;
            }
            break;
        }
        case OP_SUB: {
            if (IS_LIT(rhs, 0)) {
                node->kind = IR_ASSIGN;
            }
            break;
        }
        case OP_DIV: {
            if (IS_LIT(rhs, 1)) {
                node->kind = IR_ASSIGN;
            }
            break;
        }
        default: {
            // do nothing ...
        }
    }
}

VISIT(IR_ASSIGN) {
    if (node->lhs.id == node->tar.id && node->lhs.kind == node->tar.kind) {
        node->mark = true;
    }
}

VISIT_EMPTY(IR_BRANCH);
VISIT_EMPTY(IR_RETURN);
VISIT_EMPTY(IR_ARG);
VISIT_EMPTY(IR_WRITE);
VISIT_EMPTY(IR_STORE);
VISIT_EMPTY(IR_DREF);
VISIT_EMPTY(IR_LOAD);
VISIT_EMPTY(IR_CALL);
VISIT_EMPTY(IR_READ);
VISIT_EMPTY(IR_DEC);
VISIT_EMPTY(IR_PARAM);
VISIT_EMPTY(IR_GOTO);
VISIT_EMPTY(IR_LABEL);