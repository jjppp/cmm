#include "cfg.h"
#include "common.h"
#include "ir.h"
#include "symtab.h"
#include <stdbool.h>
#include <stdio.h>

const static char *EDGE_NAMES[] = {
    EDGES(STRING_LIST) "\0"};

static edge_t *fedge_alloc(block_t *from, block_t *to, edge_kind_t kind) {
    edge_t *e = zalloc(sizeof(edge_t));

    *e = (edge_t){
        .kind = kind,
        .from = from,
        .to   = to,
        .next = from->fedge};
    return e;
}

static edge_t *bedge_alloc(block_t *from, block_t *to) {
    edge_t *e = zalloc(sizeof(edge_t));

    *e = (edge_t){
        .from = from,
        .to   = to,
        .next = from->bedge};
    return e;
}

void edge_insert(cfg_t *cfg, block_t *from, block_t *to, edge_kind_t kind) {
    from->fedge = fedge_alloc(from, to, kind);
    to->bedge   = bedge_alloc(to, from);
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
    static u32 cnt = 1;
    ASSERT(instrs.size != 0, "empty block");
    block_t *ptr = zalloc(sizeof(block_t));

    ptr->next   = cfg->blocks;
    ptr->instrs = instrs;
    ptr->id     = cnt++;
    ir_iter(instrs.head, it) {
        it->parent = ptr;
    }

    cfg->blocks = ptr;
    cfg->nnode++;
    return ptr;
}

cfg_t *cfg_build(ir_fun_t *fun) {
    cfg_t *cfg     = zalloc(sizeof(cfg_t));
    *cfg           = (cfg_t){0};
    ir_list instrs = fun->instrs;
    symcpy(cfg->str, fun->str);
    IR_t *done = ir_alloc(IR_LABEL);

    ir_iter(instrs.head, it) {
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

    block_iter(cfg->blocks, it) {
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

void cfg_fprint(FILE *fout, const char *fname, cfg_t *cfgs) {
    fprintf(fout, "digraph program {\n");
    fprintf(fout, "label=\"%s\";\n", fname);
    fprintf(fout, "labeljust=l;\n");

    LIST_ITER(cfgs, cfg) {
        fprintf(fout, "  subgraph cluster_%s {\n", cfg->str);
        fprintf(fout, "    label=\"%s\";\n", cfg->str);
        block_iter(cfg->blocks, block) {
            fprintf(fout, "    %u [shape=box, xlabel=\"%u\", label=\"",
                    block->id, block->id);
            ir_iter(block->instrs.head, it) {
                ir_print(fout, it);
            }
            fprintf(fout, "\"];\n");
        }

        block_iter(cfg->blocks, block) {
            succ_iter(block, edge) {
                fprintf(fout, "    %u -> %u[label=\"%s\"];\n",
                        block->id, edge->to->id, EDGE_NAMES[edge->kind]);
            }
        }
        fprintf(fout, "    }\n");
    }
    fprintf(fout, "  }\n");
}