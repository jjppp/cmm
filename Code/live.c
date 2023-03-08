#include "common.h"
#include "dataflow.h"
#include "map.h"
#include "visitor.h"
#include "opt.h"
#include <stdio.h>
#include <string.h>

typedef struct live_data_t live_data_t;
#define MAGIC 0x114514
#define RET_TYPE live_data_t *
#define ARG out
VISITOR_DEF(IR, live, RET_TYPE);

struct live_data_t {
    EXTENDS(data_t);
    set_t used;
};

static void dead_check(IR_t *node, data_t *data) {
    VISITOR_DISPATCH(IR, live, node, data);
}

static void data_init(live_data_t *data) {
    data->super.magic = MAGIC;
    set_init(&data->used, oprd_cmp);
}

static void data_fini(live_data_t *data) {
    data->super.magic = MAGIC;
    set_fini(&data->used);
}

static void merge(live_data_t *into, const live_data_t *rhs) {
    ASSERT(rhs->super.magic == MAGIC, "rhs magic");
    set_merge(&into->used, &rhs->used);
}

static void gen(live_data_t *data, oprd_t oprd) {
    if (oprd.kind == OPRD_VAR) {
        set_insert(&data->used, (void *) oprd.id);
    }
}

static void kill(live_data_t *data, oprd_t oprd) {
    ASSERT(oprd.kind == OPRD_VAR, "killed OPRD_LIT");
    set_remove(&data->used, (void *) oprd.id);
}

static void *data_at(void *ptr, u32 index) {
    return &(((live_data_t *) ptr)[index]);
}

static bool data_eq(data_t *lhs, data_t *rhs) {
    return set_eq(
        &((live_data_t *) lhs)->used,
        &((live_data_t *) rhs)->used);
}

static void data_cpy(data_t *dst, data_t *src) {
    set_fini(&((live_data_t *) dst)->used);
    set_cpy(&((live_data_t *) dst)->used, &((live_data_t *) src)->used);
}

static void data_mov(data_t *dst, data_t *src) {
    data_cpy(dst, src);
}

void do_live(cfg_t *cfg) {
    dataflow df = (dataflow){
        .dir            = DF_BACKWARD,
        .merge          = (void *) merge,
        .transfer_instr = dead_check,
        .transfer_block = NULL,
        .DSIZE          = sizeof(live_data_t),
        .DMAGIC         = MAGIC,
        .data_init      = (void *) data_init,
        .data_fini      = (void *) data_fini,
        .data_at        = data_at,
        .data_eq        = data_eq,
        .data_mov       = data_mov,
        .data_cpy       = data_cpy,
        .data_in        = zalloc(sizeof(live_data_t) * cfg->nnode),
        .data_out       = zalloc(sizeof(live_data_t) * cfg->nnode)};
    LIST_ITER(cfg->blocks, blk) {
        df.data_init(df.data_at(df.data_in, blk->id));
        df.data_init(df.data_at(df.data_out, blk->id));
    }
    dataflow_init(&df);
    df.solve(cfg);

    LIST_ITER(cfg->blocks, blk) {
        live_data_t *pd = (live_data_t *) df.data_at(df.data_in, blk->id);
        LIST_REV_ITER(blk->instrs.tail, ir) {
            switch (ir->kind) {
                IR_PURE(CASE) {
                    if (!set_contains(&pd->used, (void *) ir->tar.id)) {
                        ir->mark = true;
                    }
                    break;
                }
                case IR_NULL: {
                    UNREACHABLE;
                }
                default: {
                    // do nothing ...
                }
            }
            df.transfer_instr(ir, (data_t *) pd);
        }
        ir_remove_mark(&blk->instrs);
    }
    for (u32 id = 0; id < cfg->nnode; id++) {
        data_fini(df.data_at(df.data_in, id));
        data_fini(df.data_at(df.data_out, id));
    }
    zfree(df.data_in);
    zfree(df.data_out);
}

VISIT(IR_ASSIGN) {
    kill(out, node->tar);
    gen(out, node->lhs);
}

VISIT(IR_BINARY) {
    kill(out, node->tar);
    gen(out, node->lhs);
    gen(out, node->rhs);
}

VISIT(IR_DREF) {
    kill(out, node->tar);
    gen(out, node->lhs);
}

VISIT(IR_LOAD) {
    kill(out, node->tar);
    gen(out, node->lhs);
}

VISIT(IR_STORE) {
    gen(out, node->tar);
    gen(out, node->lhs);
}

VISIT(IR_BRANCH) {
    gen(out, node->lhs);
    gen(out, node->rhs);
}

VISIT(IR_RETURN) {
    gen(out, node->lhs);
}

VISIT(IR_CALL) {
    kill(out, node->tar);
    gen(out, node->lhs);
}

VISIT(IR_READ) {
    kill(out, node->tar);
}

VISIT(IR_WRITE) {
    kill(out, node->tar);
    gen(out, node->lhs);
}

VISIT(IR_DEC) {
    kill(out, node->tar);
}

VISIT(IR_ARG) {
    gen(out, node->lhs);
}

VISIT(IR_PARAM) {
    kill(out, node->lhs);
}

VISIT_UNDEF(IR_NULL);

VISIT_EMPTY(IR_GOTO);
VISIT_EMPTY(IR_LABEL);