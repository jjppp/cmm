#pragma once
#include "cfg.h"
#include "ir.h"

#define LOCAL_OPT(F) \
    F(lvn)           \
    F(dce)           \
    F(cp_rewrite)

#define OPT_REGISTER(OPT) extern void do_##OPT(cfg_t *cfg);
#define OPT_EXECUTE(OPT) do_##OPT(cfg);

void optimize(cfg_t *cfg);