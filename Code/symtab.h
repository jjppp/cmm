#pragma once

#include "common.h"

#define MAX_SYM_LEN 64
#define MAX_SYM 128
#define MAX_CHAR 63
#define SYM_STR_SIZE (sizeof(char) * MAX_SYM_LEN)

typedef struct syment_t  syment_t;
typedef struct hashtab_t hashtab_t;

typedef enum {
    SYMS(LIST)
} sym_kind_t;

struct syment_t {
    EXTENDS(shared);
    sym_kind_t kind;
    char       str[MAX_SYM_LEN];
    u32        fst_l, fst_c;
    syment_t  *next;
};

struct hashtab_t {
    syment_t  *bucket[MAX_SYM];
    hashtab_t *prev;
};

void symtab_init();

void symtab_fini();

void *sym_insert(u32 size, const char *str, sym_kind_t kind, u32 fst_l, u32 fst_c);

void sym_scope_push();

void sym_scope_pop();

void *sym_lookup(const char *str);

void symcpy(char *dst, const char *src);

void symmov(char *dst, char *src);

int symcmp(const char *x, const char *y);

void sym_free(syment_t *sym);