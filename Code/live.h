#pragma once
#include "dataflow.h"
#include "map.h"

typedef struct live_data_t live_data_t;
struct live_data_t {
    EXTENDS(data_t);
    set_t used;
};

dataflow do_live(void *data_in, void *data_out, cfg_t *cfg);