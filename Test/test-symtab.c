#include "common.h"
#include "symtab.h"

static const char *syms[] = {
    "sym1", "arr", "struct", "_ddd", "0x12345"};
static i32 *ints[ARR_LEN(syms)] = {};

void test_insert() {
    for (u32 i = 0; i < ARR_LEN(syms); i++) {
        ints[i]  = malloc(sizeof(i32));
        *ints[i] = i;
    }
    symtab_init();
    {
        for (u32 i = 0; i < ARR_LEN(syms); i++) {
            sym_insert(syms[i], ints[i], 0, 0);
        }
        assert(sym_lookup("sym2") == NULL);
        assert(sym_lookup("sym1")->data == (void *) ints[0]);
    }
    symtab_fini();
}

void test_scope() {
    for (u32 i = 0; i < ARR_LEN(syms); i++) {
        ints[i]  = malloc(sizeof(i32));
        *ints[i] = i;
    }
    symtab_init();
    {
        sym_insert(syms[0], ints[0], 0, 0);
        sym_scope_push();
        sym_insert(syms[0], ints[1], 0, 0);
        sym_insert(syms[1], ints[2], 0, 0);
        assert(sym_lookup(syms[0])->data == (void *) ints[1]);
        assert(sym_lookup(syms[1])->data == (void *) ints[2]);
        sym_scope_pop();
        assert(sym_lookup(syms[0])->data == (void *) ints[0]);
        assert(sym_lookup(syms[1]) == NULL);
    }
    symtab_fini();
}

i32 main(void) {
    test_insert();
    test_scope();
}