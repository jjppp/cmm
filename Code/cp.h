#pragma once

#include "common.h"
#include "dataflow.h"
#include "visitor.h"
#include "ir.h"

typedef struct cp_data_t cp_data_t;
typedef struct fact_t    fact_t;
#define MAX_VARID 8192 // should be enough
#define CP_DATA_MAGIC 0x19191810
#define FACT(OPRD) ((node->OPRD.kind == OPRD_LIT) ? (const_alloc(node->OPRD.val)) : (out->fact[node->OPRD.id]))
#define FACT_TAR(OPRD) (out->fact[node->OPRD.id])
#define IS_CONST(OPRD) (FACT_TAR(OPRD).kind == FACT_CONST)

struct fact_t {
    enum {
        FACT_UNDEF = 0,
        FACT_CONST,
        FACT_NAC,
    } kind;
    u32 val;
};

struct cp_data_t {
    EXTENDS(data_t);
    fact_t fact[MAX_VARID];
};

void cp_rewrite(IR_t *ir, cp_data_t *out);