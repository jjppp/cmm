#pragma once

#include "common.h"
#include "ir.h"

#define REGS(F) \
    F($zero)    \
    F($at)      \
    F($v0)      \
    F($v1)      \
    F($a0)      \
    F($a1)      \
    F($a2)      \
    F($a3)      \
    F($t0)      \
    F($t1)      \
    F($t2)      \
    F($t3)      \
    F($t4)      \
    F($t5)      \
    F($t6)      \
    F($t7)      \
    F($s0)      \
    F($s1)      \
    F($s2)      \
    F($s3)      \
    F($s4)      \
    F($s5)      \
    F($s6)      \
    F($s7)      \
    F($t8)      \
    F($t9)      \
    F($k0)      \
    F($k1)      \
    F($gp)      \
    F($sp)      \
    F($fp)      \
    F($ra)

typedef enum {
    REGS(LIST)
} regs_t;

void mips_gen(FILE *file, ir_fun_t *prog);

void reg_alloc(ir_fun_t *fun);