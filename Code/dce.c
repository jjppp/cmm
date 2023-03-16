#include "cfg.h"
#include "common.h"
#include "dataflow.h"
#include "live.h"
#include "opt.h"
#include "reach.h"

static void remove_dead(cfg_t *cfg) {
    live_data_t *data_in  = zalloc(sizeof(live_data_t) * cfg->nnode);
    live_data_t *data_out = zalloc(sizeof(live_data_t) * cfg->nnode);

    dataflow df = do_live(data_in, data_out, cfg);
    LIST_ITER(cfg->blocks, blk) {
        live_data_t *pd = &data_in[blk->id];
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
        df.data_fini(df.data_at(df.data_in, blk->id));
        df.data_fini(df.data_at(df.data_out, blk->id));
    }
    zfree(df.data_in);
    zfree(df.data_out);
}

static void remove_unreachable(cfg_t *cfg) {
    reach_data_t *data_in  = zalloc(sizeof(reach_data_t) * cfg->nnode);
    reach_data_t *data_out = zalloc(sizeof(reach_data_t) * cfg->nnode);

    dataflow df = do_reach(data_in, data_out, cfg);
    LIST_ITER(cfg->blocks, blk) {
        blk->mark = false;
    }
    LIST_ITER(cfg->blocks, blk) {
        reach_data_t *pd = (reach_data_t *) df.data_at(df.data_in, blk->id);
        if (!pd->reachable) {
            blk->mark = true;
        }
    }
    cfg_remove_mark(cfg);
    zfree(df.data_in);
    zfree(df.data_out);
}

void do_dce(cfg_t *cfg) {
    remove_dead(cfg);
    remove_unreachable(cfg);
}