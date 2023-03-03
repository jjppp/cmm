#include "common.h"
#include "symtab.h"

static const char *syms[] = {
    "sym1", "arr", "struct", "_ddd", "0x12345"};

syment_t *ents[ARR_LEN(syms)];

void test_insert() {
    symtab_init();
    {
        for (u32 i = 0; i < ARR_LEN(syms); i++) {
            ents[i] = sym_insert(syms[i], SYM_VAR);
            assert(sym_lookup(syms[i]) == ents[i]);
        }
        assert(sym_lookup("sym2") == NULL);
        assert(sym_lookup("sym1") == ents[0]);
    }
    symtab_fini();
}

void test_scope() {
    symtab_init();
    {
        syment_t *sym0 = sym_insert(syms[0], SYM_VAR);
        sym_scope_push();
        syment_t *sym1 = sym_insert(syms[0], SYM_VAR);
        syment_t *sym2 = sym_insert(syms[1], SYM_VAR);
        assert(sym_lookup(syms[0]) == sym1);
        assert(sym_lookup(syms[1]) == sym2);
        sym_scope_pop();
        assert(sym_lookup(syms[0]) == sym0);
        assert(sym_lookup(syms[1]) == NULL);
    }
    symtab_fini();
}

i32 main(void) {
    test_insert();
    test_scope();
}