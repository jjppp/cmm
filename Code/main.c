#include <stdio.h>
#include <stdlib.h>
#include "ir.h"
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

bool check() {
    symtab_init();
    ast_check(root);
    if (sem_err) {
        return true;
    }
    return false;
}

void gen() {
    cst_print(croot, 0);
    cst_free(croot);
    ast_gen(root, NULL);
    ir_fun_print(prog);
}

#define andThen ? (done()):

i32 main(i32 argc, char **argv) {
    if (argc <= 1) {
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror(argv[1]);
        return 1;
    }
    parse(file)
        andThen
        check()
            andThen
            gen();
    return 0;
}
