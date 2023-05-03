#include <stdio.h>
#include "ast.h"
#include "cfg.h"
#include "common.h"
#include "cst.h"
#include "ir.h"
#include "mips.h"
#include "opt.h"

#define LAB3

void yyrestart(FILE *input_file);
i32  yyparse(void);

bool lex_err, syn_err, sem_err;

cfg_t    *cfgs  = NULL;
cst_t    *croot = NULL;
AST_t    *root  = NULL;
ir_fun_t *prog  = NULL;

void done() {
    ast_free(root);
}

bool parse(const char *fname) {
    FOPEN(fname, file, "r") {
        if (!file) {
            perror(fname);
            exit(1);
        }
        yyrestart(file);
        yyparse();
    }
    return (lex_err || syn_err);
}

void cst_display() {
    cst_print(croot, 0);
    cst_free(croot);
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

void gen(const char *sfname, const char *ofname) {
    ast_gen(root, var_alloc(NULL, 0));
    ir_check(&prog->instrs);

    LIST_ITER(prog, it) {
        cfg_t *cfg = cfg_build(it);
        LIST_APPEND(cfgs, cfg);
    }
    LIST_FOREACH(cfgs, optimize);
    ir_fun_free(prog);
    prog = NULL;
    LIST_ITER(cfgs, cfg) {
        ir_fun_t *fun = cfg_destruct(cfg);
        LIST_APPEND(prog, fun);
    }
    FOPEN("out.ir", file, "w") {
        if (!file) {
            perror("out.ir");
            exit(1);
        }
        ir_fun_print(file, prog);
    }
    FOPEN(ofname, file, "w") {
        if (!file) {
            perror(ofname);
            exit(1);
        }
        mips_gen(file, prog);
    }
}

#define andThen ? (done()):

i32 main(i32 argc, char **argv) {
    if (argc <= 1) {
        return 1;
    }
#ifdef LAB1
    parse(argv[1]) andThen cst_display();
#endif
#ifdef LAB2
    parse(argv[1]) andThen check();
#endif
#ifdef LAB3
    parse(argv[1]) andThen
        check() andThen
        gen(argv[1], argv[2]);
#endif
    return 0;
}
