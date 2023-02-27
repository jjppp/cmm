#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "ir.h"
#include "type.h"
#include "visitor.h"
#include "symtab.h"
#include "cst.h"

void yyrestart(FILE *input_file);
i32  yyparse(void);

bool lex_err, syn_err, sem_err;

cst_t    *croot = NULL;
AST_t    *root  = NULL;
ir_fun_t *prog  = NULL;

void done() {
    ast_free(root);
}

bool parse(FILE *file) {
    yyrestart(file);
    yyparse();
    if (lex_err || syn_err) {
        return true;
    }
    return false;
}

void lib_init() {
    static AST_t dummy = (AST_t){.kind = STMT_SCOP};

    syment_t *sym_read  = sym_insert("read", SYM_FUN);
    syment_t *sym_write = sym_insert("write", SYM_FUN);
    sym_scope_push();
    syment_t *sym_arg = sym_insert("x", SYM_VAR);
    sym_scope_pop();

    *sym_read = (syment_t){
        .kind   = SYM_FUN,
        .str    = "read",
        .body   = &dummy,
        .typ    = (type_t){.kind = TYPE_PRIM_INT},
        .nparam = 0,
        .params = NULL,
        .next   = NULL};

    *sym_arg = (syment_t){
        .kind = SYM_VAR,
        .str  = "x",
        .typ  = (type_t){.kind = TYPE_PRIM_INT},
        .next = NULL};

    *sym_write = (syment_t){
        .kind   = SYM_FUN,
        .str    = "write",
        .body   = &dummy,
        .typ    = (type_t){.kind = TYPE_PRIM_INT},
        .nparam = 1,
        .params = sym_arg,
        .next   = NULL};
}

bool check() {
    symtab_init();
    lib_init();
    ast_check(root);
    return sem_err;
}

void gen(FILE *file) {
    cst_print(croot, 0);
    cst_free(croot);
    ast_gen(root);
    ir_fun_print(file, prog);
}

#define andThen ? (done()):

i32 main(i32 argc, char **argv) {
    if (argc <= 2) {
        return 1;
    }
    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
        perror(argv[1]);
        return 1;
    }
    FILE *fout = fopen(argv[2], "w");
    parse(fin)
        andThen
        check()
            andThen
                gen(fout);
    return 0;
}
