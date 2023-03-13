#pragma once
#include "dataflow.h"
#include "map.h"
typedef struct copy_data_t copy_data_t;
struct copy_data_t {
    EXTENDS(data_t);
    set_t copy;
};

dataflow do_copy(void *data_in, void *data_out, cfg_t *cfg);