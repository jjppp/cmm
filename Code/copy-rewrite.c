#include "visitor.h"
#include "ir.h"
#include "copy.h"

#define RET_TYPE IR_t *
#define ARG copy
VISITOR_DEF(IR, copy_rewrite, RET_TYPE);

static void copy_rewrite(IR_t *ir, IR_t *copy) {
    VISITOR_DISPATCH(IR, copy_rewrite, ir, copy);
}

void do_copy_rewrite(cfg_t *cfg) {
    copy_data_t *data_in  = zalloc(sizeof(copy_data_t) * cfg->nnode);
    copy_data_t *data_out = zalloc(sizeof(copy_data_t) * cfg->nnode);

    dataflow df = do_copy(data_in, data_out, cfg);
    LIST_ITER(cfg->blocks, blk) {
        copy_data_t *pd = df.data_at(df.data_in, blk->id);
        LIST_ITER(blk->instrs.head, ir) {
            set_iter(&pd->copy, it) {
                IR_t *copy = it.val;
                copy_rewrite(ir, copy);
                if (ir->mark) {
                    ir->mark = false;
                    break;
                }
            }
            df.transfer_instr(ir, (data_t *) pd);
        }
    }
    LIST_ITER(cfg->blocks, blk) {
        df.data_fini(df.data_at(df.data_in, blk->id));
        df.data_fini(df.data_at(df.data_out, blk->id));
    }
    zfree(df.data_in);
    zfree(df.data_out);
}

static bool rewrite(oprd_t *oprd, IR_t *copy) {
    ASSERT(copy->kind == IR_ASSIGN, "copy is not IR_ASSIGN");
    if (oprd->kind == OPRD_VAR && oprd->id == copy->tar.id) {
        *oprd = copy->lhs;
        return true;
    }
    return false;
}

VISIT(IR_ASSIGN) {
    node->mark |= rewrite(&node->lhs, copy);
}

VISIT(IR_BINARY) {
    node->mark |= rewrite(&node->lhs, copy);
    node->mark |= rewrite(&node->rhs, copy);
}

VISIT(IR_BRANCH) {
    node->mark |= rewrite(&node->lhs, copy);
    node->mark |= rewrite(&node->rhs, copy);
}

VISIT(IR_RETURN) {
    node->mark |= rewrite(&node->lhs, copy);
}

VISIT(IR_ARG) {
    node->mark |= rewrite(&node->lhs, copy);
}

VISIT(IR_WRITE) {
    node->mark |= rewrite(&node->lhs, copy);
}

VISIT(IR_STORE) {
    node->mark |= rewrite(&node->lhs, copy);
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