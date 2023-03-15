#include "common.h"
#include "dataflow.h"
#include "def.h"
#include "ir.h"
#include "map.h"
#include "visitor.h"
#include "opt.h"
#include "live.h"
#include <stdio.h>
#include <string.h>

#define MAGIC 998244353
#define RET_TYPE def_data_t *
#define ARG out
VISITOR_DEF(IR, def, RET_TYPE);

static mapent_t entries[65536];
static dataflow live_df;

static void def_check(IR_t *node, data_t *data) {
    VISITOR_DISPATCH(IR, def, node, data);
}

static void transfer_block(block_t *blk, data_t *data_in) {
    LIST_ITER(blk->instrs.head, ir) {
        def_check(ir, data_in);
    }
    live_data_t *pd = live_df.data_at(live_df.data_in, blk->id);

    set_t *defs = &((def_data_t *) data_in)->defs;
    u32    size = map_to_array(defs, entries);
    for (u32 i = 0; i < size; i++) {
        const IR_t *ir = entries[i].val;
        if (!set_contains(&pd->used, (void *) ir->tar.id)) { // remove dead defs
            set_remove(defs, ir);
        }
    }
}

static void data_init(def_data_t *data) {
    data->super.magic = MAGIC;
    set_init(&data->defs);
}

static void data_fini(def_data_t *data) {
    data->super.magic = MAGIC;
    set_fini(&data->defs);
}

static bool merge(def_data_t *into, const def_data_t *rhs) {
    ASSERT(rhs->super.magic == MAGIC, "rhs magic");
    return set_merge(&into->defs, &rhs->defs);
}

static void *data_at(void *ptr, u32 index) {
    return &(((def_data_t *) ptr)[index]);
}

static bool data_eq(data_t *lhs, data_t *rhs) {
    return set_eq(
        &((def_data_t *) lhs)->defs,
        &((def_data_t *) rhs)->defs);
}

static void data_cpy(data_t *dst, data_t *src) {
    set_fini(&((def_data_t *) dst)->defs);
    set_cpy(&((def_data_t *) dst)->defs, &((def_data_t *) src)->defs);
}

static void data_mov(data_t *dst, data_t *src) {
    swap(((def_data_t *) dst)->defs, ((def_data_t *) src)->defs);
}

dataflow do_def(void *data_in, void *data_out, cfg_t *cfg) {
    live_data_t *live_data_in  = zalloc(sizeof(live_data_t) * cfg->nnode);
    live_data_t *live_data_out = zalloc(sizeof(live_data_t) * cfg->nnode);

    live_df = do_live(live_data_in, live_data_out, cfg);

    dataflow df = (dataflow){
        .dir            = DF_FORWARD,
        .merge          = (void *) merge,
        .transfer_instr = def_check,
        .transfer_block = transfer_block,
        .DSIZE          = sizeof(def_data_t),
        .DMAGIC         = MAGIC,
        .data_init      = (void *) data_init,
        .data_fini      = (void *) data_fini,
        .data_at        = data_at,
        .data_eq        = data_eq,
        .data_mov       = data_mov,
        .data_cpy       = data_cpy,
        .data_in        = data_in,
        .data_out       = data_out};
    LIST_ITER(cfg->blocks, blk) {
        df.data_init(df.data_at(df.data_in, blk->id));
        df.data_init(df.data_at(df.data_out, blk->id));
    }
    dataflow_init(&df);
    df.solve(cfg);

    LIST_ITER(cfg->blocks, blk) {
        live_df.data_fini(live_df.data_at(live_df.data_in, blk->id));
        live_df.data_fini(live_df.data_at(live_df.data_out, blk->id));
    }
    zfree(live_data_in);
    zfree(live_data_out);
    return df;
}

static void kill(oprd_t oprd, set_t *defs) {
    u32 size = map_to_array(defs, entries);
    for (u32 i = 0; i < size; i++) {
        IR_t *ir = entries[i].val;
        if (ir->tar.id == oprd.id) {
            set_remove(defs, ir);
        }
    }
}

static void gen(set_t *defs, IR_t *ir) {
    set_insert(defs, ir);
}

VISIT(IR_ASSIGN) {
    kill(node->tar, &out->defs);
    gen(&out->defs, node);
}

VISIT(IR_BINARY) {
    kill(node->tar, &out->defs);
    gen(&out->defs, node);
}

VISIT(IR_DREF) {
    kill(node->tar, &out->defs);
    gen(&out->defs, node);
}

VISIT(IR_LOAD) {
    kill(node->tar, &out->defs);
    gen(&out->defs, node);
}

VISIT(IR_CALL) {
    kill(node->tar, &out->defs);
    gen(&out->defs, node);
}

VISIT(IR_READ) {
    kill(node->tar, &out->defs);
    gen(&out->defs, node);
}

VISIT(IR_WRITE) {
    kill(node->tar, &out->defs);
    gen(&out->defs, node);
}

VISIT(IR_DEC) {
    kill(node->tar, &out->defs);
    gen(&out->defs, node);
}

VISIT(IR_PARAM) {
    kill(node->tar, &out->defs);
    gen(&out->defs, node);
}

VISIT_UNDEF(IR_NULL);

VISIT_EMPTY(IR_GOTO);
VISIT_EMPTY(IR_LABEL);
VISIT_EMPTY(IR_STORE);
VISIT_EMPTY(IR_BRANCH);
VISIT_EMPTY(IR_RETURN);
VISIT_EMPTY(IR_ARG);