#pragma once

#include "common.h"
#include "dataflow.h"
#include "map.h"
#include "visitor.h"
#include "opt.h"

typedef struct cp_data_t cp_data_t;
typedef struct fact_t    fact_t;
#define MAGIC 0x19191810

struct fact_t {
    union {
        struct {
            enum {
                FACT_UNDEF = 0,
                FACT_CONST,
                FACT_NAC,
            } kind;
            u32 val;
        };
        uptr rep;
    };
};

struct cp_data_t {
    EXTENDS(data_t);
    map_t facts;
};

void cp_rewrite(IR_t *ir, cp_data_t *out);

void fact_insert(cp_data_t *out, oprd_t oprd, fact_t fact);

fact_t fact_get(cp_data_t *out, oprd_t oprd);