#include "symtab.h"
#include "ast.h"
#include "common.h"
#include "hashtab.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static symtab_t *top  = NULL;
static bool      init = false;
static syment_t  entries[MAX_SYM];

syment_t *sym_lookup(const char *str) {
    ASSERT(init, "symtab used before initialized");
    LIST_ITER(top, cur) {
        hashent_t *ent = hash_lookup(&cur->hashtab, str);
        syment_t  *sym = ent->ptr;
        if (sym && !strcmp(sym->str, str)) {
            return sym;
        }
    }
    return NULL;
}

void sym_scope_push() {
    ASSERT(init, "symtab used before initialized");
    symtab_t *symtab = zalloc(sizeof(symtab_t));
    symtab->next     = top;
    top              = symtab;
}

void sym_scope_pop() {
    ASSERT(init, "symtab used before initialized");
    symtab_t *symtab = top;
    top              = top->next;
    zfree(symtab);
}

void *salloc(u32 size) {
    static syment_t *ptr = entries;
    ASSERT(ptr - entries != MAX_SYM, "entries overflow");
    return ptr++;
}

void *sym_insert(const char *str, sym_kind_t kind) {
    ASSERT(init, "symtab used before initialized");
    ASSERT(strlen(str) < MAX_SYM_LEN, "sym_insert exceeds MAX_SYM_LEN");
    hashent_t *ent = hash_lookup(&top->hashtab, str);
    if (ent->ptr != NULL) {
        return NULL;
    }

    syment_t *sym = salloc(sizeof(syment_t));
    *sym          = (syment_t){.kind = kind};
    symcpy(sym->str, str);

    ent->ptr = sym;
    ent->str = sym->str;
    return sym;
}

void symtab_init() {
    init = true;
    sym_scope_push();
}

void symtab_fini() {
    while (top) {
        sym_scope_pop();
    }
    init = false;
}

void symcpy(char *dst, const char *src) {
    strncpy(dst, src, MAX_SYM_LEN - 1);
}

void symmov(char *dst, char *src) {
    symcpy(dst, src);
    zfree(src);
}

i32 symcmp(const char *x, const char *y) {
    return strncmp(x, y, MAX_SYM_LEN);
}

char *symuniq(const char *suffix) {
    static u32 cnt = 0;
    char      *str = zalloc(MAX_SYM_LEN * sizeof(char));
    snprintf(str, MAX_SYM_LEN - 1, "%u_%s", cnt, suffix);
    cnt++;
    return str;
}