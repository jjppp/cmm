#include "cfg.h"
#include "common.h"
#include "ir.h"
#include "symtab.h"
#include <stdbool.h>
#include <stdio.h>

const static char *EDGE_NAMES[] = {
    EDGES(STRING_LIST) "\0"};
static u32 blk_cnt;

static edge_t *fedge_alloc(block_t *from, block_t *to, edge_kind_t kind) {
    edge_t *e = zalloc(sizeof(edge_t));

    *e = (edge_t){
        .kind = kind,
        .from = from,
        .to   = to,
        .next = from->fedge};
    return e;
}

static edge_t *bedge_alloc(edge_t *fedge) {
    edge_t *e = zalloc(sizeof(edge_t));

    *e = (edge_t){
        .kind = fedge->kind,
        .from = fedge->to,
        .to   = fedge->from,
        .next = fedge->to->bedge,
        .rev  = fedge};
    return e;
}

void edge_insert(cfg_t *cfg, block_t *from, block_t *to, edge_kind_t kind) {
    from->fedge      = fedge_alloc(from, to, kind);
    to->bedge        = bedge_alloc(from->fedge);
    from->fedge->rev = to->bedge;
}

static inline bool is_term(IR_t *ir) {
    if (ir == NULL) {
        return false;
    }
    switch (ir->kind) {
        case IR_BRANCH:
        case IR_RETURN:
        case IR_GOTO: return true;
        default: return false;
    }
    UNREACHABLE;
}

static inline bool is_start(IR_t *ir) {
    if (ir == NULL) {
        return false;
    }
    return (ir->kind == IR_LABEL);
}

static inline bool is_fall(IR_t *ir) {
    if (ir == NULL) {
        return false;
    }
    return (ir->kind != IR_GOTO && ir->kind != IR_RETURN);
}

block_t *block_alloc(cfg_t *cfg, ir_list instrs) {
    ASSERT(instrs.size != 0, "empty block");
    block_t *ptr = zalloc(sizeof(block_t));

    ptr->next   = cfg->blocks;
    ptr->instrs = instrs;
    ptr->id     = blk_cnt++;
    LIST_ITER(instrs.head, it) {
        it->parent = ptr;
    }

    cfg->blocks = ptr;
    cfg->nnode++;
    if (ptr->next == NULL) {
        cfg->entry = ptr;
    }
    return ptr;
}

static void block_free(block_t *blk) {
    ir_list_free(&blk->instrs);
    zfree(blk);
}

cfg_t *cfg_build(ir_fun_t *fun) {
    cfg_t *cfg     = zalloc(sizeof(cfg_t));
    *cfg           = (cfg_t){0};
    ir_list instrs = fun->instrs;
    symcpy(cfg->str, fun->str);
    IR_t *done = ir_alloc(IR_LABEL);
    blk_cnt    = 0;

    LIST_ITER(instrs.head, it) {
        if (is_start(it) || is_term(it->prev)) {
            ir_list front = ir_split(&instrs, it);
            block_alloc(cfg, front);
        }
        if (it->kind == IR_RETURN) {
            it->jmpto = done;
        }
    }

    if (instrs.size != 0) {
        block_alloc(cfg, instrs);
    }
    done->parent = block_alloc(cfg, (ir_list){.head = done, .tail = done, .size = 1, .var = {0}});
    cfg->exit    = done->parent;

    LIST_ITER(cfg->blocks, it) {
        IR_t *tail = it->instrs.tail;
        if (it->next != NULL && is_fall(it->next->instrs.tail)) {
            edge_insert(cfg, it->next, it, EDGE_THROUGH);
        }
        if (is_term(tail)) {
            block_t *to = tail->jmpto->parent;
            switch (tail->kind) {
                case IR_GOTO: {
                    edge_insert(cfg, it, to, EDGE_GOTO);
                    break;
                }
                case IR_BRANCH: {
                    edge_insert(cfg, it, to, EDGE_TRUE);
                    break;
                }
                case IR_RETURN: {
                    edge_insert(cfg, it, to, EDGE_RETURN);
                    break;
                }
                default: UNREACHABLE;
            }
        }
    }
    return cfg;
}

static void dfs1(block_t *blk, ir_list *list) {
    if (blk->mark) {
        return;
    }
    blk->mark = true;
    ir_concat(list, blk->instrs);
    succ_iter(blk, e) {
        if (e->kind == EDGE_THROUGH) {
            dfs1(e->to, list);
        }
    }
}

static void dfs2(block_t *blk, ir_list *list) {
    if (blk->mark) {
        return;
    }
    blk->mark = true;
    ir_concat(list, blk->instrs);
    succ_iter(blk, e) {
        dfs2(e->to, list);
    }
}

static bool is_leading(block_t *blk) {
    pred_iter(blk, e) {
        if (e->kind == EDGE_THROUGH) {
            return false;
        }
    }
    return true;
}

ir_fun_t *cfg_destruct(cfg_t *cfg) { // TODO: mem leak
    ir_fun_t *fun    = zalloc(sizeof(ir_fun_t));
    ir_list  *instrs = &fun->instrs;
    symcpy(fun->str, cfg->str);
    LIST_ITER(cfg->blocks, blk) {
        blk->mark = false;
    }
    cfg->exit->mark = true;
    dfs1(cfg->entry, instrs);
    LIST_ITER(cfg->blocks, blk) {
        if (is_leading(blk)) {
            dfs1(blk, instrs);
        }
    }
    LIST_ITER(cfg->blocks, blk) {
        dfs2(blk, instrs);
    }
    return fun;
}

void cfg_remove_mark(cfg_t *cfg) {
#define EDGE_MARK(EDGE) (((EDGE)->to->mark) || ((EDGE)->from->mark))

    LIST_ITER(cfg->blocks, blk) {
        LIST_REMOVE(blk->fedge, zfree, EDGE_MARK);
        LIST_REMOVE(blk->bedge, zfree, EDGE_MARK);
    }
    LIST_REMOVE(cfg->blocks, block_free, MARKED);

    LIST_ITER(cfg->blocks, blk) {
        if (blk->mark) {
            abort();
        }
        succ_iter(blk, e) {
            if (e->to->mark) {
                abort();
            }
        }
        pred_iter(blk, e) {
            if (e->to->mark) {
                abort();
            }
        }
    }
}

void cfg_fprint(FILE *fout, const char *fname, cfg_t *cfgs) {
    fprintf(fout, "digraph program {\n");
    fprintf(fout, "label=\"%s\";\n", fname);
    fprintf(fout, "labeljust=l;\n");
    u32 cnt = 0;

    LIST_ITER(cfgs, cfg) {
        u32 *pid = zalloc(sizeof(u32) * cfg->nnode);
        LIST_ITER(cfg->blocks, blk) {
            pid[blk->id] = cnt++;
        }

        fprintf(fout, "  subgraph cluster_%s {\n", cfg->str);
        fprintf(fout, "    label=\"%s\";\n", cfg->str);
        LIST_ITER(cfg->blocks, block) {
            fprintf(fout, "    %u [shape=box, xlabel=\"%u\", label=\"",
                    pid[block->id], pid[block->id]);
            LIST_ITER(block->instrs.head, it) {
                ir_print(fout, it);
            }
            fprintf(fout, "\"];\n");
        }

        LIST_ITER(cfg->blocks, block) {
            succ_iter(block, edge) {
                fprintf(fout, "    %u -> %u[label=\"%s\"];\n",
                        pid[block->id], pid[edge->to->id], EDGE_NAMES[edge->kind]);
            }
        }
        fprintf(fout, "    }\n");
        zfree(pid);
    }
    fprintf(fout, "  }\n");
}