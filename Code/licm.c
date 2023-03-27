#include "common.h"
#include "dom.h"
#include "def.h"
#include "cfg.h"
#include "ir.h"
#include "map.h"
#include <string.h>

typedef struct loop_t {
    block_t *hdr, *pre_hdr;
    set_t    blocks;
} loop_t;

static bool *vis;

static bool outof_loop(oprd_t oprd, set_t *defs, loop_t *loop) {
    if (oprd.kind == OPRD_LIT) {
        return true;
    }
    set_iter(defs, it) {
        IR_t *ir = it.val;
        if (ir->tar.id == oprd.id) {
            if (set_contains(&loop->blocks, ir->parent)) {
                return false;
            }
        }
    }
    return true;
}

static bool inv_check(IR_t *ir, set_t *defs, loop_t *loop) {
    switch (ir->kind) {
        case IR_BINARY: return (outof_loop(ir->lhs, defs, loop) && outof_loop(ir->rhs, defs, loop));
        // TODO: still buggy
        case IR_ASSIGN: return outof_loop(ir->lhs, defs, loop);
        default: return false;
    }
    UNREACHABLE;
}

static bool can_hoist(IR_t *ir, loop_t *loop) {
    return false;
}

static void loop_init(loop_t *loop, block_t *hdr) {
    set_init(&loop->blocks);
    set_insert(&loop->blocks, hdr);
    loop->hdr     = hdr;
    loop->pre_hdr = NULL;
}

static void loop_fini(loop_t *loop) {
    set_fini(&loop->blocks);
}

static void set_pre_hdr(loop_t *loop) {
    loop->pre_hdr = NULL;
    pred_iter(loop->hdr, e) {
        block_t *blk = e->to;
        if (!set_contains(&loop->blocks, blk)) {
            blk->mark = true;
            ASSERT(loop->pre_hdr == NULL, "PRE_HEADER not unique");
            loop->pre_hdr = blk;
        }
    }
    ASSERT(loop->pre_hdr != NULL, "PRE_HEADER");
}

static void dfs(block_t *blk, block_t *dom, loop_t *cur_loop) {
    if (blk == dom || vis[blk->id]) return;
    vis[blk->id] = true;
    set_insert(&cur_loop->blocks, blk);
    pred_iter(blk, e) {
        dfs(e->to, dom, cur_loop);
    }
}

static void mark_inv(loop_t *loop, dataflow df) {
    set_iter(&loop->blocks, it) {
        block_t *blk = it.val;
        LIST_ITER(blk->instrs.head, ir) {
            def_data_t *pd = df.data_at(df.data_in, blk->id);

            if (inv_check(ir, &pd->defs, loop)
                && can_hoist(ir, loop)) {
                ir->parent = loop->pre_hdr;
            }
            df.transfer_instr(ir, pd);
        }
    }
}

void do_licm(cfg_t *cfg) {
    dom_data_t *dom_in  = zalloc(sizeof(dom_data_t) * cfg->nnode);
    dom_data_t *dom_out = zalloc(sizeof(dom_data_t) * cfg->nnode);
    def_data_t *def_in  = zalloc(sizeof(def_data_t) * cfg->nnode);
    def_data_t *def_out = zalloc(sizeof(def_data_t) * cfg->nnode);

    dataflow dom_df = do_dom(dom_in, dom_out, cfg);
    dataflow def_df = do_def(def_in, def_out, cfg);
    vis             = zalloc(sizeof(bool) * cfg->nnode);
    LIST_ITER(cfg->blocks, blk) {
        dom_data_t *pd = dom_df.data_at(dom_df.data_out, blk->id);
        succ_iter(blk, e) {
            if (set_contains(&pd->dom, e->to)) {
                loop_t loop;
                loop_init(&loop, e->to);
                memset(vis, 0, sizeof(bool) * cfg->nnode);
                dfs(blk, e->to, &loop);
                set_pre_hdr(&loop);
                mark_inv(&loop, def_df);
                loop_fini(&loop);
            }
        }
    }
    LIST_ITER(cfg->blocks, blk) {
        LIST_ITER(blk->instrs.head, ir) {
            if (ir->parent != blk) {
                ir_append(&ir->parent->instrs, ir_dup(ir));
                ir->mark = true;
            }
        }
    }
    LIST_ITER(cfg->blocks, blk) {
        ir_remove_mark(&blk->instrs);
    }
    LIST_ITER(cfg->blocks, blk) {
        def_df.data_fini(def_df.data_at(def_df.data_in, blk->id));
        def_df.data_fini(def_df.data_at(def_df.data_out, blk->id));
        dom_df.data_fini(dom_df.data_at(dom_df.data_in, blk->id));
        dom_df.data_fini(dom_df.data_at(dom_df.data_out, blk->id));
    }
    zfree(vis);
    zfree(def_in);
    zfree(def_out);
    zfree(dom_in);
    zfree(dom_out);
}