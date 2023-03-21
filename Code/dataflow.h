#pragma once
#include "cfg.h"

typedef struct dataflow dataflow;

typedef enum {
    DF_FORWARD,
    DF_BACKWARD,
} df_dir_t;

struct dataflow {
    void (*transfer_instr)(IR_t *ir, void *data_in);
    void (*transfer_block)(block_t *blk, void *data_in);
    bool (*merge)(void *into, const void *other);
    void (*solve)(cfg_t *cfg);
    void (*data_init)(void *data);
    void (*data_fini)(void *data);
    void *(*data_at)(void *base, u32 index);
    bool (*data_eq)(void *lhs, void *rhs);
    void (*data_mov)(void *lhs, void *rhs);
    void (*data_cpy)(void *lhs, void *rhs);
    void    *data_in, *data_out;
    df_dir_t dir;
    u32      DSIZE;
};

void dataflow_init(dataflow *df_init);