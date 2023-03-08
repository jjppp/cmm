#include "common.h"
#include "dataflow.h"
#include "opt.h"
#include <stdio.h>
#include <string.h>

typedef struct dce_data_t dce_data_t;
#define MAGIC 0x19260817

struct dce_data_t {
    EXTENDS(data_t);
    bool reachable;
};

static void dce_check(block_t *blk, data_t *data) {
    ASSERT(data->magic == MAGIC, "dce magic");
}

static void data_init(dce_data_t *data) {
    data->super.magic = MAGIC;
    data->reachable   = false;
}

static void merge(dce_data_t *into, const dce_data_t *rhs) {
    ASSERT(rhs->super.magic == MAGIC, "rhs magic");
    into->reachable |= rhs->reachable;
}

static void *data_at(void *ptr, u32 index) {
    ASSERT(index < MAX_VARID, "data overflow");
    return &(((dce_data_t *) ptr)[index]);
}

static bool data_eq(data_t *lhs, data_t *rhs) {
    return ((dce_data_t *) lhs)->reachable == ((dce_data_t *) rhs)->reachable;
}

static void data_cpy(data_t *dst, data_t *src) {
    ((dce_data_t *) dst)->reachable = ((dce_data_t *) src)->reachable;
}

static void data_mov(data_t *dst, data_t *src) {
    data_cpy(dst, src);
}

void do_dce(cfg_t *cfg) {
    dataflow df = (dataflow){
        .dir            = DF_FORWARD,
        .merge          = (void *) merge,
        .transfer_instr = NULL,
        .transfer_block = dce_check,
        .DSIZE          = sizeof(dce_data_t),
        .DMAGIC         = MAGIC,
        .data_init      = (void *) data_init,
        .data_at        = data_at,
        .data_eq        = data_eq,
        .data_cpy       = data_cpy,
        .data_mov       = data_mov,
        .data_in        = zalloc(sizeof(dce_data_t) * cfg->nnode),
        .data_out       = zalloc(sizeof(dce_data_t) * cfg->nnode)};
    LIST_ITER(cfg->blocks, blk) {
        df.data_init(df.data_at(df.data_in, blk->id));
        df.data_init(df.data_at(df.data_out, blk->id));
        blk->mark = false;
    }
    ((dce_data_t *) df.data_at(df.data_in, cfg->entry->id))->reachable = true;
    dataflow_init(&df);
    df.solve(cfg);

    LIST_ITER(cfg->blocks, blk) {
        dce_data_t *pd = (dce_data_t *) df.data_at(df.data_in, blk->id);
        if (!pd->reachable) {
            blk->mark = true;
        }
    }
    cfg_remove_mark(cfg);
    zfree(df.data_in);
    zfree(df.data_out);
}