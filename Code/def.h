#pragma once
#include "dataflow.h"
#include "map.h"

typedef struct def_data_t def_data_t;
struct def_data_t {
    EXTENDS(data_t);
    set_t defs;
};

dataflow do_def(void *data_in, void *data_out, cfg_t *cfg);