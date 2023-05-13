#include <stdio.h>
#include "cfg.h"
#include "common.h"
#include "ir.h"
#include "opt.h"

#define LAB3

void yyrestart(FILE *input_file);
i32  yyparse(void);

bool lex_err, syn_err, sem_err;

cfg_t    *cfgs = NULL;
ir_fun_t *prog = NULL;

i32 main(i32 argc, char **argv) {
    if (argc <= 2) {
        return 1;
    }
    FOPEN(argv[1], file, "r", {
        yyrestart(file);
        yyparse();
    });
    if (lex_err || syn_err) {
        return 1;
    }
    ir_resolve(prog);

    // LIST_MAP_REDUCE(prog, cfg_build, cfgs);
    // LIST_FOREACH(cfgs, optimize);
    // prog = NULL;
    // LIST_MAP_REDUCE(cfgs, cfg_destruct, prog);
    FOPEN(argv[2], file, "w", {
        ir_fun_print(file, prog);
    });
    return 0;
}
