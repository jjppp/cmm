#include "cfg.h"
#include "cp.h"
#include <stdio.h>
#include <string.h>

#define RET_TYPE cp_data_t *
#define ARG out
VISITOR_DEF(IR, cp, RET_TYPE);

static fact_t NAC = (fact_t){.kind = FACT_NAC};

static void const_prop(IR_t *node, data_t *data) {
    VISITOR_DISPATCH(IR, cp, node, data);
}

static void data_init(cp_data_t *data) {
    data->super.magic = LIVE_DATA_MAGIC;
    memset(data->fact, 0, sizeof(data->fact));
}

static fact_t const_alloc(u32 val) {
    return (fact_t){.kind = FACT_CONST, .val = val};
}

static fact_t fact_merge(const fact_t lhs, const fact_t rhs) {
    if (lhs.kind == FACT_NAC || rhs.kind == FACT_NAC) {
        return NAC;
    }
    if (lhs.kind == FACT_UNDEF) {
        return rhs;
    }
    if (rhs.kind == FACT_UNDEF) {
        return lhs;
    }
    return (lhs.val == rhs.val) ? lhs : NAC;
}

static fact_t fact_compute(op_kind_t op, const fact_t lhs, const fact_t rhs) {
    if (lhs.kind == FACT_NAC || rhs.kind == FACT_NAC) return NAC;
    if (lhs.kind == FACT_UNDEF) return rhs;
    if (rhs.kind == FACT_UNDEF) return lhs;
    switch (op) {
        case OP_ADD: return const_alloc(lhs.val + rhs.val);
        case OP_SUB: return const_alloc(lhs.val - rhs.val);
        case OP_MUL: return const_alloc(lhs.val * rhs.val);
        case OP_DIV: return const_alloc(lhs.val / rhs.val);
        default: UNREACHABLE;
    }
    UNREACHABLE;
}

static void merge(cp_data_t *into, const cp_data_t *rhs) {
    ASSERT(rhs->super.magic == LIVE_DATA_MAGIC, "rhs magic");
    for (u32 i = 0; i < MAX_VARID; i++) {
        into->fact[i] = fact_merge(into->fact[i], rhs->fact[i]);
    }
}

static void *data_at(void *ptr, u32 index) {
    return &(((cp_data_t *) ptr)[index]);
}

void do_cp(cfg_t *cfg) {
    dataflow df = (dataflow){
        .dir            = DF_FORWARD,
        .merge          = (void *) merge,
        .transfer_instr = const_prop,
        .transfer_block = NULL,
        .DSIZE          = sizeof(cp_data_t),
        .DMAGIC         = LIVE_DATA_MAGIC,
        .data_init      = (void *) data_init,
        .data_at        = data_at,
        .data_in        = zalloc(sizeof(cp_data_t) * cfg->nnode),
        .data_out       = zalloc(sizeof(cp_data_t) * cfg->nnode)};
    LIST_ITER(cfg->blocks, blk) {
        df.data_init(df.data_at(df.data_in, blk->id));
        df.data_init(df.data_at(df.data_out, blk->id));
    }
    dataflow_init(&df);
    df.solve(cfg);

    LIST_ITER(cfg->blocks, blk) {
        cp_data_t *pd = (cp_data_t *) df.data_at(df.data_in, blk->id);
        LIST_ITER(blk->instrs.head, ir) {
            df.transfer_instr(ir, (data_t *) pd);
            cp_rewrite(ir, pd);
        }
        // ir_remove_mark(&blk->instrs);
    }
    zfree(df.data_in);
    zfree(df.data_out);
}

VISIT(IR_ASSIGN) {
    FACT_TAR(tar) = FACT(lhs);
}

VISIT(IR_BINARY) {
    FACT_TAR(tar) = fact_compute(node->op, FACT(lhs), FACT(rhs));
}

VISIT(IR_DREF) {
    FACT_TAR(tar) = NAC;
}

VISIT(IR_LOAD) {
    FACT_TAR(tar) = NAC;
}

VISIT(IR_CALL) {
    FACT_TAR(tar) = NAC;
}

VISIT(IR_READ) {
    FACT_TAR(tar) = NAC;
}

VISIT(IR_WRITE) {
    FACT_TAR(tar) = const_alloc(0);
}

VISIT(IR_DEC) {
    FACT_TAR(tar) = NAC;
}

VISIT(IR_PARAM) { // intra-procedural, safe
    FACT_TAR(lhs) = NAC;
}

VISIT_UNDEF(IR_NULL);

VISIT_EMPTY(IR_GOTO);
VISIT_EMPTY(IR_LABEL);
VISIT_EMPTY(IR_BRANCH);
VISIT_EMPTY(IR_RETURN);
VISIT_EMPTY(IR_STORE);
VISIT_EMPTY(IR_ARG);