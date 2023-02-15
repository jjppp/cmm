#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

void yyrestart(FILE *input_file);
int  yyparse(void);

bool lex_err, syn_err, sem_err;

cst_t *croot = NULL;
ast_t *root  = NULL;

int main(int argc, char **argv) {
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
    if (!lex_err && !syn_err) {
        puts("NO ERR");
        cst_print(croot, 0);
        cst_free(croot);
    }
    ast_free(root);
    return 0;
}
