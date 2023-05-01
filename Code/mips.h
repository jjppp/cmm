#pragma once

#include "common.h"

#define REGS(F) \
    F(ZERO)     \
    F(AT)       \
    F(V0)       \
    F(V1)       \
    F(A0)       \
    F(A1)       \
    F(A2)       \
    F(A3)       \
    F(T0)       \
    F(T1)       \
    F(T2)       \
    F(T3)       \
    F(T4)       \
    F(T5)       \
    F(T6)       \
    F(T7)       \
    F(S0)       \
    F(S1)       \
    F(S2)       \
    F(S3)       \
    F(S4)       \
    F(S5)       \
    F(S6)       \
    F(S7)       \
    F(T8)       \
    F(T9)       \
    F(K0)       \
    F(K1)       \
    F(GP)       \
    F(SP)       \
    F(FP)       \
    F(RA)

typedef enum {
    REGS(LIST)
} regs_t;