#pragma once
#include "cfg.h"
#include "ir.h"

#define MAX_VARID 65536

#define LOCAL_OPT(F) \
    F(lvn)           \
    F(live)          \
    F(cp)            \
    F(dce)

#define OPT_REGISTER(OPT) extern void do_##OPT(cfg_t *cfg);
#define OPT_EXECUTE(OPT) do_##OPT(cfg);

void optimize(cfg_t *cfg);