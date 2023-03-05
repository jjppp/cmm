#include "dataflow.h"
#include "cfg.h"
#include "common.h"
#include <string.h>

#define MAX_QUEUE_LEN 4096 // should be enough

static block_t  *queue[MAX_QUEUE_LEN];
static bool      inq[MAX_QUEUE_LEN];
static u32       head, tail;
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

void dataflow_init(dataflow *df_init) { // TODO: mem leak
    memset(inq, 0, sizeof(inq));
    memset(queue, 0, sizeof(queue));
    head = 0;
    tail = 0;
    df   = df_init;
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

static void push(block_t *blk) {
    if (inq[blk->id]) return;
    ASSERT(tail < MAX_QUEUE_LEN, "queue overflow");
    inq[blk->id]  = true;
    queue[tail++] = blk;
}

static block_t *pop() {
    block_t *blk = queue[head++];
    inq[blk->id] = false;
    return blk;
}

static bool empty() {
    return head >= tail;
}

static void dataflow_bsolve(cfg_t *cfg) { // backward
    LIST_FOREACH(cfg->blocks, push);
    data_t *newd = zalloc(df->DSIZE);
    newd->magic  = df->DMAGIC;
    while (!empty()) {
        block_t *blk = pop();
        LOG("%u\n", blk->id);
        df->data_init(newd);
        succ_iter(blk, e) {
            df->merge(newd, df->data_at(df->data_out, e->to->id));
            data_validate(newd);
        }
        memcpy(df->data_at(df->data_in, blk->id), newd, df->DSIZE);
        df->transfer_block(blk, newd);
        if (memcmp(df->data_at(df->data_out, blk->id), newd, df->DSIZE)) {
            memcpy(df->data_at(df->data_out, blk->id), newd, df->DSIZE);
            pred_foreach(blk, push);
        }
    }
    zfree(newd);
}

static void dataflow_fsolve(cfg_t *cfg) { // forward
    LIST_FOREACH(cfg->blocks, push);
    data_t *newd = zalloc(df->DSIZE);
    newd->magic  = df->DMAGIC;
    while (!empty()) {
        block_t *blk = pop();
        LOG("%u\n", blk->id);
        df->data_init(df->data_at(df->data_in, blk->id));
        pred_iter(blk, e) {
            df->merge(df->data_at(df->data_in, blk->id), df->data_at(df->data_out, e->to->id));
        }
        memcpy(newd, df->data_at(df->data_in, blk->id), df->DSIZE);
        df->transfer_block(blk, newd);
        if (memcmp(df->data_at(df->data_out, blk->id), newd, df->DSIZE)) {
            memmove(df->data_at(df->data_out, blk->id), newd, df->DSIZE);
            succ_foreach(blk, push);
        }
    }
    zfree(newd);
}
