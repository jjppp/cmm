#pragma once

#include "common.h"
#include "symtab.h"

typedef struct cst_t cst_t;

#define cst_foreach(NODE, IT) \
    for (cst_t *IT = (NODE)->chld; IT != NULL; IT = IT->next)

struct cst_t {
    char   str[MAX_SYM_LEN];
    char   typ[MAX_SYM_LEN];
    cst_t *chld, *next;
    u32    fst_l;
    bool   is_tok;
};

cst_t *cst_alloc(const char *typ, const char *name, u32 fst_line, u32 nchld, ...);

void cst_free(cst_t *node);

void cst_print(cst_t *node, i32 dep);