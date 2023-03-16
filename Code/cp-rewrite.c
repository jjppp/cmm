#include "cp.h"
#include "ir.h"

#define RET_TYPE cp_data_t *
#define ARG out
VISITOR_DEF(IR, cp_rewrite, RET_TYPE);

static void cp_rewrite(IR_t *ir, cp_data_t *out) {
    ASSERT(out->super.magic == MAGIC, "data magic");
    VISITOR_DISPATCH(IR, cp_rewrite, ir, out);
}

void do_cp_rewrite(cfg_t *cfg) {
    cp_data_t *data_in  = zalloc(sizeof(cp_data_t) * cfg->nnode);
    cp_data_t *data_out = zalloc(sizeof(cp_data_t) * cfg->nnode);

    dataflow df = do_cp(data_in, data_out, cfg);
    LIST_ITER(cfg->blocks, blk) {
        cp_data_t *pd = (cp_data_t *) df.data_at(df.data_in, blk->id);
        LIST_ITER(blk->instrs.head, ir) {
            df.transfer_instr(ir, (data_t *) pd);
            cp_rewrite(ir, pd);
        }
        // ir_remove_mark(&blk->instrs);
        df.data_fini(df.data_at(df.data_in, blk->id));
        df.data_fini(df.data_at(df.data_out, blk->id));
    }
    zfree(df.data_in);
    zfree(df.data_out);
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