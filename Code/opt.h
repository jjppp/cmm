#pragma once
#include "cfg.h"
#include "ir.h"

#define LOCAL_OPT(F) \
    F(lvn)

#define OPT_REGISTER(OPT) \
    extern void do_##OPT(cfg_t *cfg);

void optimize(cfg_t *cfg);