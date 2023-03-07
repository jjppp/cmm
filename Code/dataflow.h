#pragma once
#include "cfg.h"

typedef struct dataflow dataflow;
typedef struct data_t   data_t;

typedef enum {
    DF_FORWARD,
    DF_BACKWARD,
} df_dir_t;

struct data_t {
    u32 magic;
};

struct dataflow {
    void (*transfer_instr)(IR_t *ir, data_t *data_in);
    void (*transfer_block)(block_t *blk, data_t *data_in);
    void (*merge)(data_t *into, const data_t *other);
    void (*solve)(cfg_t *cfg);
    void (*data_init)(data_t *data);
    void *(*data_at)(void *base, u32 index);
    bool (*data_eq)(data_t *lhs, data_t *rhs);
    void (*data_mov)(data_t *lhs, data_t *rhs);
    void (*data_cpy)(data_t *lhs, data_t *rhs);
    void    *data_in, *data_out;
    df_dir_t dir;
    u32      DSIZE, DMAGIC;
};

void dataflow_init(dataflow *df_init);