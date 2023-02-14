#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

void yyrestart(FILE *input_file);
int  yyparse(void);

bool lex_err, syn_err, sem_err;

ast_t *root = NULL;

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
        void print(FILE * file, ast_t * node, ...);
        print(stdout, root);
    }
    del_ast_node(root);
    return 0;
}
