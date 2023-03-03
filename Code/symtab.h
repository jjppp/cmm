#pragma once

#include "common.h"
#include "hashtab.h"
#include "type.h"
#include "ir.h"

typedef struct syment_t syment_t;
typedef struct symtab_t symtab_t;

struct symtab_t {
    hashtab_t        hashtab;
    struct symtab_t *next;
};

typedef enum {
    SYMS(LIST)
} sym_kind_t;

struct syment_t {
    EXTENDS(shared);
    sym_kind_t    kind;
    char          str[MAX_SYM_LEN];
    type_t        typ;
    syment_t     *next, *params;
    u32           nparam;
    struct AST_t *body;
    struct oprd_t var;
};

void symtab_init();

void symtab_fini();

void *sym_insert(const char *str, sym_kind_t kind);

void sym_scope_push();

void sym_scope_pop();

syment_t *sym_lookup(const char *str);

void symcpy(char *dst, const char *src);

void symmov(char *dst, char *src);

i32 symcmp(const char *x, const char *y);

char *symuniq(const char *);