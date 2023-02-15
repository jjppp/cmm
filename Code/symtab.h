#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include "common.h"
#include <stdbool.h>

#define MAX_SYM_LEN 64
#define MAX_SYM 128
#define MAX_CHAR 63
#define SYM_STR_SIZE (sizeof(char) * MAX_SYM_LEN)

typedef struct __syment_t  syment_t;
typedef struct __hashtab_t hashtab_t;

struct __syment_t {
    char  str[MAX_SYM_LEN];
    void *data; // own, copy before use
    u32   fst_l, fst_c;
};

struct __hashtab_t {
    syment_t  *bucket[MAX_SYM];
    hashtab_t *prev;
};

void symtab_init();

void symtab_fini();

// moves data into syment
void sym_insert(const char *str, void *data, u32 fst_l, u32 fst_c);

void sym_scope_push();

void sym_scope_pop();

syment_t *sym_lookup(const char *str);

void symcpy(char *dst, const char *src);

void symmov(char *dst, char *src);

int symcmp(const char *x, const char *y);

#endif