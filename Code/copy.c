#include "cfg.h"
#include "common.h"
#include "copy.h"
#include "ir.h"
#include "visitor.h"
#include "opt.h"
#include "map.h"
#include <stdio.h>
#include <string.h>

#define MAGIC 0x210093
#define RET_TYPE copy_data_t *
#define ARG out
VISITOR_DEF(IR, copy, RET_TYPE);

static set_t UNIVERSE;

static void kill(oprd_t oprd, set_t *copy) {
    static mapent_t entries[65536];

    u32 len = map_to_array(copy, entries);
    for (u32 i = 0; i < len; i++) {
        IR_t *ir = entries[i].val;
        if (ir->tar.id == oprd.id || ir->lhs.id == oprd.id) {
            set_remove(copy, ir);
        }
    }
}

static void transfer(IR_t *ir, data_t *out) {
    VISITOR_DISPATCH(IR, copy, ir, ARG);
}

static void data_init(copy_data_t *data) {
    data->super.magic = MAGIC;
    set_cpy(&data->copy, &UNIVERSE);
}

static void data_fini(copy_data_t *data) {
    data->super.magic = MAGIC;
    set_fini(&data->copy);
}

static bool merge(copy_data_t *into, const copy_data_t *rhs) {
    ASSERT(rhs->super.magic == MAGIC, "rhs magic");
    return set_intersect(&into->copy, &rhs->copy);
}

static void *data_at(void *ptr, u32 index) {
    return &(((copy_data_t *) ptr)[index]);
}

static bool data_eq(data_t *lhs, data_t *rhs) {
    return set_eq(
        &((copy_data_t *) lhs)->copy,
        &((copy_data_t *) rhs)->copy);
}

static void data_cpy(data_t *dst, data_t *src) {
    set_fini(&((copy_data_t *) dst)->copy);
    set_cpy(&((copy_data_t *) dst)->copy, &((copy_data_t *) src)->copy);
}

static void data_mov(data_t *dst, data_t *src) {
    swap(((copy_data_t *) dst)->copy, ((copy_data_t *) src)->copy);
}

dataflow do_copy(void *data_in, void *data_out, cfg_t *cfg) {
    dataflow df = (dataflow){
        .dir            = DF_FORWARD,
        .merge          = (void *) merge,
        .transfer_instr = transfer,
        .transfer_block = NULL,
        .DSIZE          = sizeof(copy_data_t),
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
        LIST_ITER(blk->instrs.head, ir) {
            if (ir->kind == IR_ASSIGN) {
                set_insert(&UNIVERSE, ir);
            }
        }
    }
    LIST_ITER(cfg->blocks, blk) {
        if (blk == cfg->entry) {
            set_init(&((copy_data_t *) df.data_at(df.data_in, cfg->entry->id))->copy);
        } else {
            df.data_init(df.data_at(df.data_in, blk->id));
        }
    }
    dataflow_init(&df);
    df.solve(cfg);
    set_fini(&UNIVERSE);
    return df;
}

VISIT(IR_ASSIGN) {
    kill(node->tar, &out->copy);
    set_insert(&out->copy, node);
}

VISIT(IR_BINARY) {
    kill(node->tar, &out->copy);
}

VISIT(IR_DREF) {
    kill(node->tar, &out->copy);
}

VISIT(IR_LOAD) {
    kill(node->tar, &out->copy);
}

VISIT(IR_CALL) {
    kill(node->tar, &out->copy);
}

VISIT(IR_READ) {
    kill(node->tar, &out->copy);
}

VISIT(IR_WRITE) {
    kill(node->tar, &out->copy);
}

VISIT(IR_DEC) {
    kill(node->tar, &out->copy);
}

VISIT(IR_PARAM) {
    kill(node->tar, &out->copy);
}

VISIT_UNDEF(IR_NULL);

VISIT_EMPTY(IR_GOTO);
VISIT_EMPTY(IR_LABEL);
VISIT_EMPTY(IR_STORE);
VISIT_EMPTY(IR_BRANCH);
VISIT_EMPTY(IR_RETURN);
VISIT_EMPTY(IR_ARG);