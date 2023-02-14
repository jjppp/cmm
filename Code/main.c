#include <stdio.h>
#include <stdlib.h>
#include "symtab.h"
#include "ast.h"

void yyrestart(FILE *input_file);

int yyparse(void);

bool lexical_error_detected;
bool syntax_error_detected;

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
    // extern int yydebug;
    // yydebug = 1;
    yyrestart(file);
    yyparse();
    if (!lexical_error_detected && !syntax_error_detected) {
        puts("NO ERR");
        void print(FILE * file, ast_t * node, ...);
        print(stdout, root);
        // PrintTree(tree_root, 0);
    }
    return 0;
}
