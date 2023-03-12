#include "opt.h"
#include "cfg.h"

#define NROUND 5

LOCAL_OPT(OPT_REGISTER)

extern void do_licm(cfg_t *cfg);

void optimize(cfg_t *cfg) {
    LOCAL_OPT(OPT_EXECUTE)
    do_licm(cfg);
    LOCAL_OPT(OPT_EXECUTE)
}