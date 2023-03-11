#include "dataflow.h"
#include "cfg.h"
#include "common.h"
#include "queue.h"
#include <string.h>

static queue_t   que;
static dataflow *df;

static void data_validate(const data_t *data) {
    ASSERT(data->magic == df->DMAGIC, "not a data");
}

static void dataflow_bsolve(cfg_t *cfg);
static void dataflow_fsolve(cfg_t *cfg);

static void transfer_fblock(block_t *blk, data_t *data_in) { // forward
    data_validate(data_in);
    LIST_ITER(blk->instrs.head, it) {
        df->transfer_instr(it, data_in);
    }
}

static void transfer_bblock(block_t *blk, data_t *data_in) { // backward
    data_validate(data_in);
    LIST_REV_ITER(blk->instrs.tail, it) {
        df->transfer_instr(it, data_in);
    }
}

void dataflow_init(dataflow *df_init) {
    df = df_init;
    ASSERT(df->DSIZE > sizeof(data_t), "df->data_size(%u) <= sizeof(data_t)", df->DSIZE);
    ASSERT(df->DMAGIC != 0, "df->DMAGIC == 0");
    switch (df->dir) {
        case DF_FORWARD: {
            df->solve = dataflow_fsolve;
            if (df->transfer_block == NULL) {
                df->transfer_block = transfer_fblock;
            }
            break;
        }
        case DF_BACKWARD: {
            df->solve = dataflow_bsolve;
            if (df->transfer_block == NULL) {
                df->transfer_block = transfer_bblock;
            }
            break;
        }
        default: UNREACHABLE;
    }
}

static void dataflow_bsolve(cfg_t *cfg) { // backward
    queue_init(&que, cfg->nnode);
    LIST_ITER(cfg->blocks, blk) {
        df->data_cpy(df->data_at(df->data_out, blk->id), df->data_at(df->data_in, blk->id));
        df->transfer_block(blk, df->data_at(df->data_out, blk->id));
        queue_push(&que, blk);
    }
    data_t *newd = zalloc(df->DSIZE);
    df->data_init(newd);
    while (!queue_empty(&que)) {
        block_t *blk = queue_pop(&que);
        LOG("%u\n", blk->id);
        bool changed = false;
        succ_iter(blk, e) {
            changed |= df->merge(df->data_at(df->data_in, blk->id), df->data_at(df->data_out, e->to->id));
        }
        if (!changed) continue;
        df->data_cpy(newd, df->data_at(df->data_in, blk->id));
        df->transfer_block(blk, newd);
        if (!df->data_eq(df->data_at(df->data_out, blk->id), newd)) {
            df->data_mov(df->data_at(df->data_out, blk->id), newd);
            pred_iter(blk, it) {
                queue_push(&que, it->to);
            }
        }
    }
    queue_fini(&que);
    df->data_fini(newd);
    zfree(newd);
}

static void dataflow_fsolve(cfg_t *cfg) { // forward
    queue_init(&que, cfg->nnode);
    LIST_ITER(cfg->blocks, blk) {
        df->data_cpy(df->data_at(df->data_out, blk->id), df->data_at(df->data_in, blk->id));
        df->transfer_block(blk, df->data_at(df->data_out, blk->id));
        queue_push_front(&que, blk);
    }
    data_t *newd = zalloc(df->DSIZE);
    df->data_init(newd);
    while (!queue_empty(&que)) {
        block_t *blk = queue_pop(&que);
        LOG("%u\n", blk->id);
        bool changed = false;
        pred_iter(blk, e) {
            changed |= df->merge(df->data_at(df->data_in, blk->id), df->data_at(df->data_out, e->to->id));
        }
        if (!changed) continue;
        df->data_cpy(newd, df->data_at(df->data_in, blk->id));
        df->transfer_block(blk, newd);
        if (!df->data_eq(df->data_at(df->data_out, blk->id), newd)) {
            df->data_mov(df->data_at(df->data_out, blk->id), newd);
            succ_iter(blk, it) {
                queue_push(&que, it->to);
            }
        }
    }
    queue_fini(&que);
    df->data_fini(newd);
    zfree(newd);
}
