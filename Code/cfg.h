#pragma once

#include "common.h"
#include "ir.h"

typedef struct block_t block_t;
typedef struct edge_t  edge_t;
typedef struct cfg_t   cfg_t;

#define succ_iter(BLOCK, IT) LIST_ITER((BLOCK)->fedge, (IT))
#define succ_foreach(BLOCK, FUN) \
    LIST_ITER((BLOCK)->fedge, __it) { FUN(__it->to); }

#define pred_iter(BLOCK, IT) LIST_ITER((BLOCK)->bedge, (IT))
#define pred_foreach(BLOCK, FUN) \
    LIST_ITER((BLOCK)->bedge, __it) { FUN(__it->to); }

struct cfg_t {
    block_t *blocks, *entry, *exit;
    cfg_t   *next;

    u32  nnode, nedge;
    char str[MAX_SYM_LEN];
};

#define EDGES(F)   \
    F(EDGE_NULL)   \
    F(EDGE_GOTO)   \
    F(EDGE_TRUE)   \
    F(EDGE_RETURN) \
    F(EDGE_THROUGH)

typedef enum {
    EDGES(LIST)
} edge_kind_t;

struct edge_t {
    bool        mark;
    edge_kind_t kind;
    block_t    *from, *to;
    edge_t     *next, *rev;
};

struct block_t {
    edge_t  *fedge, *bedge;
    ir_list  instrs;
    block_t *next;
    u32      id;
    bool     mark;
};

void edge_insert(cfg_t *cfg, block_t *from, block_t *to, edge_kind_t kind);

cfg_t *cfg_build(ir_fun_t *fun);

ir_fun_t *cfg_destruct(cfg_t *cfg);

void cfg_fprint(FILE *fout, const char *fname, cfg_t *cfg);

void cfg_remove_mark(cfg_t *cfg);