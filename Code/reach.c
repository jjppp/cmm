#include "reach.h"
#include "cfg.h"

static void dce_check(block_t *blk, void *data) {
}

static void data_init(reach_data_t *data) {
    data->reachable = false;
}

static void data_fini(reach_data_t *data) {
}

static bool merge(reach_data_t *into, const reach_data_t *rhs) {
    into->reachable |= rhs->reachable;
    return true;
}

static void *data_at(void *ptr, u32 index) {
    return &(((reach_data_t *) ptr)[index]);
}

static bool data_eq(void *lhs, void *rhs) {
    return ((reach_data_t *) lhs)->reachable == ((reach_data_t *) rhs)->reachable;
}

static void data_cpy(void *dst, void *src) {
    ((reach_data_t *) dst)->reachable = ((reach_data_t *) src)->reachable;
}

static void data_mov(void *dst, void *src) {
    data_cpy(dst, src);
}

dataflow do_reach(void *data_in, void *data_out, cfg_t *cfg) {
    dataflow df = (dataflow){
        .dir            = DF_FORWARD,
        .merge          = (void *) merge,
        .transfer_instr = NULL,
        .transfer_block = dce_check,
        .DSIZE          = sizeof(reach_data_t),
        .data_init      = (void *) data_init,
        .data_fini      = (void *) data_fini,
        .data_at        = data_at,
        .data_eq        = data_eq,
        .data_cpy       = data_cpy,
        .data_mov       = data_mov,
        .data_in        = data_in,
        .data_out       = data_out};
    LIST_ITER(cfg->blocks, blk) {
        df.data_init(df.data_at(df.data_in, blk->id));
        blk->mark = false;
    }
    ((reach_data_t *) df.data_at(df.data_in, cfg->entry->id))->reachable = true;
    dataflow_init(&df);
    df.solve(cfg);
    return df;
}