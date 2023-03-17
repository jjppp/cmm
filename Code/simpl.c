#include "visitor.h"
#include "cfg.h"
#include "ir.h"

#define RET_TYPE cfg_t *
#define ARG cfg
VISITOR_DEF(IR, simpl, RET_TYPE);

static void simpl(cfg_t *cfg, IR_t *ir) {
    VISITOR_DISPATCH(IR, simpl, ir, cfg);
}

void do_simpl(cfg_t *cfg) {
    LIST_ITER(cfg->blocks, blk) {
        LIST_ITER(blk->instrs.head, ir) {
            simpl(cfg, ir);
        }
        ir_remove_mark(&blk->instrs);
    }
    edge_remove_mark(cfg);
}

static bool take(IR_t *node) {
    i32 lhs_val = node->lhs.val;
    i32 rhs_val = node->rhs.val;
    switch (node->op) {
        case OP_LE: return lhs_val <= rhs_val;
        case OP_LT: return lhs_val < rhs_val;
        case OP_GE: return lhs_val >= rhs_val;
        case OP_GT: return lhs_val > rhs_val;
        case OP_EQ: return lhs_val == rhs_val;
        case OP_NE: return lhs_val != rhs_val;
        default: UNREACHABLE;
    }
}

static bool const_branch(IR_t *node) {
    return (node->lhs.kind == OPRD_LIT && node->rhs.kind == OPRD_LIT);
}

VISIT(IR_BRANCH) {
    if (const_branch(node)) {
        node->kind  = IR_GOTO;
        IR_t *jmpto = node->jmpto;
        if (take(node)) {
            succ_iter(node->parent, e) {
                if (e->to != jmpto->parent) {
                    e->mark = true;
                } else {
                    e->kind = EDGE_GOTO;
                }
            }
        } else {
            node->mark = true;
            succ_iter(node->parent, e) {
                if (e->to == jmpto->parent) {
                    e->mark = true;
                } else {
                    e->kind = EDGE_THROUGH;
                }
            }
        }
    } else {
        block_t *tar_blk = node->jmpto->parent;
        ir_list *instrs  = &tar_blk->instrs;
        if (instrs->size == 2 && instrs->head->next->kind == IR_GOTO) {
            node->jmpto = instrs->head->next->jmpto;
            succ_iter(node->parent, e) {
                if (e->to == tar_blk) {
                    e->mark = true;
                }
            }
            edge_insert(cfg, node->parent, node->jmpto->parent, EDGE_GOTO);
        }
    }
}

VISIT(IR_GOTO) {
    block_t *tar_blk = node->jmpto->parent;
    ir_list *instrs  = &tar_blk->instrs;
    if (instrs->size == 2 && instrs->head->next->kind == IR_GOTO) {
        node->jmpto = instrs->head->next->jmpto;
        succ_iter(node->parent, e) {
            if (e->to == tar_blk) {
                e->mark = true;
            }
        }
        edge_insert(cfg, node->parent, node->jmpto->parent, EDGE_GOTO);
    }
}

VISIT_UNDEF(IR_NULL);

VISIT_EMPTY(IR_RETURN);
VISIT_EMPTY(IR_ASSIGN);
VISIT_EMPTY(IR_BINARY);
VISIT_EMPTY(IR_WRITE);
VISIT_EMPTY(IR_ARG);
VISIT_EMPTY(IR_STORE);
VISIT_EMPTY(IR_DREF);
VISIT_EMPTY(IR_LOAD);
VISIT_EMPTY(IR_CALL);
VISIT_EMPTY(IR_READ);
VISIT_EMPTY(IR_DEC);
VISIT_EMPTY(IR_PARAM);
VISIT_EMPTY(IR_LABEL);