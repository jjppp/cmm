#include "opt.h"
#include "cfg.h"

LOCAL_OPT(OPT_REGISTER)

void optimize(cfg_t *cfg) {
    do_lvn(cfg);
}