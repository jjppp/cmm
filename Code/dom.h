#pragma once
#include "dataflow.h"
#include "map.h"

typedef struct dom_data_t dom_data_t;
struct dom_data_t {
    set_t dom;
};

dataflow do_dom(void *data_in, void *data_out, cfg_t *cfg);