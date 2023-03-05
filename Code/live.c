#include "ast.h"
#include "cfg.h"
#include "common.h"
#include "ir.h"
#include "visitor.h"
#include "symtab.h"
#include <stdio.h>
#include <string.h>

typedef struct data_t data_t;
#define RET_TYPE data_t *
#define ARG out
VISITOR_DEF(IR, live, RET_TYPE);

#define MAX_INSTR 8192 // should be enough

static bool     inq[MAX_INSTR];
static block_t *queue[MAX_INSTR];
static struct data_t {
    bool used[MAX_INSTR];
} data_in[MAX_INSTR], data_out[MAX_INSTR];
static u32 head, tail;

static void live_init() { // TODO: mem leak
    memset(inq, 0, sizeof(inq));
    memset(queue, 0, sizeof(queue));
    memset(data_in, 0, sizeof(data_in));
    memset(data_out, 0, sizeof(data_out));
    head = tail = 0;
}

static void push(block_t *blk) {
    if (inq[blk->id]) return;
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

static void dead_check(IR_t *node, data_t *data) {
    VISITOR_DISPATCH(IR, live, node, data);
}

static void merge(data_t *into, data_t *rhs) {
    for (u32 i = 0; i < MAX_INSTR; i++) {
        into->used[i] |= rhs->used[i];
    }
}

static void transfer(block_t *blk, data_t *newd) {
    LIST_REV_ITER(blk->instrs.tail, it) {
        dead_check(it, newd);
    }
}

void do_live(cfg_t *cfg) {
    live_init();
    LIST_FOREACH(cfg->blocks, push);

    data_t *newd = zalloc(sizeof(data_t));
    while (!empty()) {
        block_t *blk = pop();
        succ_iter(blk, e) {
            merge(&data_in[blk->id], &data_out[e->to->id]);
        }
        memcpy(newd, &data_in[blk->id], sizeof(data_t));
        transfer(blk, newd);
        if (memcmp(&data_out[blk->id], newd, sizeof(data_t))) {
            memmove(&data_out[blk->id], newd, sizeof(data_t));
            pred_iter(blk, e) {
                push(e->to);
            }
        }
    }
    zfree(newd);

    LIST_ITER(cfg->blocks, blk) {
        data_t *pd = &data_in[blk->id];
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
            dead_check(ir, pd);
        }
        ir_remove_mark(&blk->instrs);
    }
}

VISIT(IR_ASSIGN) {
    if (node->lhs.kind == OPRD_VAR) {
        out->used[node->lhs.id] = true;
    }
    out->used[node->tar.id] = false;
}

VISIT(IR_BINARY) {
    if (node->lhs.kind == OPRD_VAR) {
        out->used[node->lhs.id] = true;
    }
    if (node->rhs.kind == OPRD_VAR) {
        out->used[node->rhs.id] = true;
    }
    out->used[node->tar.id] = false;
}

VISIT(IR_DREF) {
    out->used[node->lhs.id] = true;
    out->used[node->tar.id] = false;
}

VISIT(IR_LOAD) {
    out->used[node->lhs.id] = true;
    out->used[node->tar.id] = false;
}

VISIT(IR_STORE) {
    if (node->lhs.kind == OPRD_VAR) {
        out->used[node->lhs.id] = true;
    }
    out->used[node->tar.id] = false;
}

VISIT(IR_BRANCH) {
    if (node->lhs.kind == OPRD_VAR) {
        out->used[node->lhs.id] = true;
    }
    if (node->rhs.kind == OPRD_VAR) {
        out->used[node->rhs.id] = true;
    }
}

VISIT(IR_RETURN) {
    if (node->lhs.kind == OPRD_VAR) {
        out->used[node->lhs.id] = true;
    }
}

VISIT(IR_CALL) {
    if (node->lhs.kind == OPRD_VAR) {
        out->used[node->lhs.id] = true;
    }
    out->used[node->tar.id] = false;
}

VISIT(IR_READ) {
    out->used[node->tar.id] = false;
}

VISIT(IR_WRITE) {
    if (node->lhs.kind == OPRD_VAR) {
        out->used[node->lhs.id] = true;
    }
    out->used[node->tar.id] = false;
}

VISIT(IR_DEC) {
    out->used[node->tar.id] = false;
}

VISIT(IR_ARG) {
    if (node->lhs.kind == OPRD_VAR) {
        out->used[node->lhs.id] = true;
    }
}

VISIT(IR_PARAM) {
    out->used[node->lhs.id] = false;
}

VISIT_UNDEF(IR_NULL);

VISIT_EMPTY(IR_GOTO);
VISIT_EMPTY(IR_LABEL);