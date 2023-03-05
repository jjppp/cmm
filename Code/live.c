#include "ast.h"
#include "cfg.h"
#include "common.h"
#include "dataflow.h"
#include "ir.h"
#include "visitor.h"
#include "symtab.h"
#include <stdio.h>
#include <string.h>

typedef struct live_data_t live_data_t;
#define MAX_VARID 8192 // should be enough
#define LIVE_DATA_MAGIC 0x114514
#define RET_TYPE live_data_t *
#define ARG out
VISITOR_DEF(IR, live, RET_TYPE);

struct live_data_t {
    EXTENDS(data_t);
    bool used[MAX_VARID];
};

static void dead_check(IR_t *node, data_t *data) {
    VISITOR_DISPATCH(IR, live, node, data);
}

static void data_init(live_data_t *data) {
    data->super.magic = LIVE_DATA_MAGIC;
    memset(data->used, 0, sizeof(data->used));
}

static void merge(live_data_t *into, const live_data_t *rhs) {
    ASSERT(rhs->super.magic == LIVE_DATA_MAGIC, "rhs magic");
    for (u32 i = 0; i < MAX_VARID; i++) {
        into->used[i] |= rhs->used[i];
    }
}

static void gen(live_data_t *data, oprd_t oprd) {
    if (oprd.kind == OPRD_VAR) {
        data->used[oprd.id] = true;
    }
}

static void kill(live_data_t *data, oprd_t oprd) {
    ASSERT(oprd.kind == OPRD_VAR, "killed OPRD_LIT");
    data->used[oprd.id] = false;
}

static void *data_at(void *ptr, u32 index) {
    return &(((live_data_t *) ptr)[index]);
}

void do_live(cfg_t *cfg) {
    dataflow df = (dataflow){
        .dir            = DF_BACKWARD,
        .merge          = (void *) merge,
        .transfer_instr = dead_check,
        .transfer_block = NULL,
        .DSIZE          = sizeof(live_data_t),
        .DMAGIC         = LIVE_DATA_MAGIC,
        .data_init      = (void *) data_init,
        .data_at        = data_at,
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
                    if (!pd->used[ir->tar.id]) {
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