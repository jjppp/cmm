#include <stdio.h>
#include <stdlib.h>
#include "visitor.h"
#include "symtab.h"
#include "cst.h"

void yyrestart(FILE *input_file);
i32  yyparse(void);

bool lex_err, syn_err, sem_err;

cst_t *croot = NULL;
ast_t *root  = NULL;

i32 main(i32 argc, char **argv) {
    if (argc <= 1) {
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror(argv[1]);
        return 1;
    }
    yyrestart(file);
    yyparse();
    if (lex_err || syn_err) {
        goto done;
    }
    symtab_init();
    ast_check(root);
    if (sem_err) {
        goto done;
    }
    cst_print(croot, 0);
    cst_free(croot);
done:
    ast_free(root);
    return 0;
}
