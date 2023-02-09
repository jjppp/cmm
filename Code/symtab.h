#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include "common.h"
#include <stdbool.h>

#define MAX_SYM_LEN 16
#define MAX_SYM 128
#define MAX_CHAR 63

typedef struct __syment_t  syment_t;
typedef struct __hashtab_t hashtab_t;

struct __syment_t {
    char  str[MAX_SYM_LEN];
    void *data; // own, copy before use
    u32   fst_l, fst_c;
    bool  valid;
};

struct __hashtab_t {
    syment_t   bucket[MAX_SYM];
    hashtab_t *prev;
};

void symtab_init();

void symtab_fini();

// moves data into syment
void sym_insert(const char *str, void *data, u32 fst_l, u32 fst_c);

void sym_scope_push();

void sym_scope_pop();

syment_t *sym_lookup(const char *str);

#endif