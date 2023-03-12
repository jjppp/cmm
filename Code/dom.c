#include "cfg.h"
#include "common.h"
#include "visitor.h"
#include "opt.h"
#include "dom.h"
#include <stdio.h>
#include <string.h>

#define MAGIC 0x20020308

static set_t UNIVERSE;

static void transfer(block_t *blk, data_t *data) {
    set_insert(&((dom_data_t *) data)->dom, blk);
}

static void data_init(dom_data_t *data) {
    data->super.magic = MAGIC;
    set_cpy(&data->dom, &UNIVERSE);
}

static void data_fini(dom_data_t *data) {
    data->super.magic = MAGIC;
    set_fini(&data->dom);
}

static bool merge(dom_data_t *into, const dom_data_t *rhs) {
    ASSERT(rhs->super.magic == MAGIC, "rhs magic");
    map_intersect(&into->dom, &rhs->dom);
    return true;
}

static void *data_at(void *ptr, u32 index) {
    return &(((dom_data_t *) ptr)[index]);
}

static bool data_eq(data_t *lhs, data_t *rhs) {
    return set_eq(
        &((dom_data_t *) lhs)->dom,
        &((dom_data_t *) rhs)->dom);
}

static void data_cpy(data_t *dst, data_t *src) {
    set_fini(&((dom_data_t *) dst)->dom);
    set_cpy(&((dom_data_t *) dst)->dom, &((dom_data_t *) src)->dom);
}

static void data_mov(data_t *dst, data_t *src) {
    swap(((dom_data_t *) dst)->dom, ((dom_data_t *) src)->dom);
}

dataflow do_dom(void *data_in, void *data_out, cfg_t *cfg) {
    dataflow df = (dataflow){
        .dir            = DF_FORWARD,
        .merge          = (void *) merge,
        .transfer_instr = NULL,
        .transfer_block = transfer,
        .DSIZE          = sizeof(dom_data_t),
        .DMAGIC         = MAGIC,
        .data_init      = (void *) data_init,
        .data_fini      = (void *) data_fini,
        .data_at        = data_at,
        .data_eq        = data_eq,
        .data_mov       = data_mov,
        .data_cpy       = data_cpy,
        .data_in        = data_in,
        .data_out       = data_out};

    set_init(&UNIVERSE);
    LIST_ITER(cfg->blocks, blk) {
        set_insert(&UNIVERSE, blk);
    }
    LIST_ITER(cfg->blocks, blk) {
        if (blk == cfg->entry) {
            set_init(&((dom_data_t *) df.data_at(df.data_in, cfg->entry->id))->dom);
            set_insert(&((dom_data_t *) df.data_at(df.data_in, cfg->entry->id))->dom, cfg->entry);
        } else {
            df.data_init(df.data_at(df.data_in, blk->id));
        }
        df.data_init(df.data_at(df.data_out, blk->id));
    }
    dataflow_init(&df);
    df.solve(cfg);
    return df;
}