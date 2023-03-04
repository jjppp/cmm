#include "opt.h"
#include "cfg.h"

#define NROUND 5

LOCAL_OPT(OPT_REGISTER)

void optimize(cfg_t *cfg) {
    LOCAL_OPT(OPT_EXECUTE);
}