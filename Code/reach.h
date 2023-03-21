#pragma once
#include "dataflow.h"

typedef struct reach_data_t reach_data_t;
struct reach_data_t {
    bool reachable;
};

dataflow do_reach(void *data_in, void *data_out, cfg_t *cfg);