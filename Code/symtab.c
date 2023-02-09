#include "symtab.h"
#include <string.h>
#include <assert.h>

static hashtab_t *top  = NULL;
static bool       init = false;

static u32 str_hash(const char *s) {
    u32 hash = 0;
    for (const char *p = s; *p; p++) {
        hash = (hash * 17 + *p) % MAX_SYM;
    }
    return hash;
}

static syment_t *lookup(hashtab_t *hashtab, const char *str) {
    syment_t *bucket = hashtab->bucket;
    u32       hash   = str_hash(str);
    while (bucket[hash].valid && strcmp(bucket[hash].str, str)) {
        hash = (hash + 1) % MAX_SYM;
    }
    return &bucket[hash];
}

syment_t *sym_lookup(const char *str) {
    assert(init);
    for (hashtab_t *cur = top; cur != NULL; cur = cur->prev) {
        syment_t *sym = lookup(cur, str);
        if (!strcmp(sym->str, str)) {
            return sym;
        }
    }
    return NULL;
}

void sym_scope_push() {
    assert(init);
    hashtab_t *hashtab = zalloc(sizeof(hashtab_t));
    hashtab->prev      = top;
    top                = hashtab;
}

void sym_scope_pop() {
    assert(init);
    for (u32 i = 0; i < MAX_SYM; i++) {
        if (top->bucket[i].valid) {
            free(top->bucket[i].data);
        }
    }
    hashtab_t *hashtab = top;
    top                = top->prev;
    free(hashtab);
}

void sym_insert(const char *str, void *data, u32 fst_l, u32 fst_c) {
    assert(init);
    assert(strlen(str) < MAX_SYM_LEN);
    syment_t *sym = lookup(top, str);
    assert(!sym->valid);

    *sym = (syment_t){
        .data  = data,
        .fst_c = fst_c,
        .fst_l = fst_l,
        .valid = true};
    memcpy(sym->str, str, strlen(str));
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