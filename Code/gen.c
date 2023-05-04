#include "ast.h"
#include "ir.h"
#include "common.h"
#include "type.h"
#include "visitor.h"
#include "symtab.h"

#define RET_TYPE ir_list *
#define ARG list
VISITOR_DEF(AST, gen, RET_TYPE);

ir_list lexpr_gen(AST_t *node, oprd_t tar);
ir_list cond_gen(AST_t *node);

oprd_t oprd_stack[32768]; // should be enough
u32    oprd_top = 0;

oprd_t oprd_tar() {
    return oprd_stack[oprd_top];
}

void oprd_push(oprd_t oprd) {
    oprd_stack[++oprd_top] = oprd;
}

void oprd_pop() {
    oprd_top--;
}

ir_list ast_gen(AST_t *node, oprd_t tar) {
    oprd_push(tar);
    ir_list ARG;
    VISITOR_DISPATCH(AST, gen, node, &ARG);
    oprd_pop();
    return ARG;
}

VISIT(EXPR_INT) {
    ir_list lit = {0};
    ir_append(&lit, ir_alloc(IR_ASSIGN, oprd_tar(), lit_alloc(node->value)));
    RETURN(lit);
}

VISIT(EXPR_BIN) {
    ir_list lhs = {0}, rhs = {0};
    switch (node->op) {
        REL_OPS(CASE)
        LOGIC_OPS(CASE) {
            ir_list result = cond_gen((AST_t *) node);

            IR_t *ltru = ir_alloc(IR_LABEL);
            IR_t *lfls = ir_alloc(IR_LABEL);
            IR_t *done = ir_alloc(IR_LABEL);
            chain_resolve(&result.tru, ltru);
            chain_resolve(&result.fls, lfls);
            ir_append(&result, ltru);
            ir_append(&result, ir_alloc(IR_ASSIGN, oprd_tar(), lit_alloc(1)));
            ir_append(&result, ir_alloc(IR_GOTO, done));
            ir_append(&result, lfls);
            ir_append(&result, ir_alloc(IR_ASSIGN, oprd_tar(), lit_alloc(0)));
            ir_append(&result, done);
            RETURN(result);
        }
        ARITH_OPS(CASE) {
            oprd_t lhs_var = var_alloc(NULL, node->super.fst_l);
            oprd_t rhs_var = var_alloc(NULL, node->super.fst_l);

            lhs = ast_gen(node->lhs, lhs_var);
            rhs = ast_gen(node->rhs, rhs_var);
            ir_concat(&lhs, rhs);
            ir_append(&lhs, ir_alloc(IR_BINARY, node->op, oprd_tar(), lhs_var, rhs_var));
            RETURN(lhs);
        }
        default: UNREACHABLE;
    }
    UNREACHABLE;
}

VISIT(EXPR_UNR) {
    switch (node->op) {
        case OP_NOT: {
            ir_list result = cond_gen((AST_t *) node);

            IR_t *ltru = ir_alloc(IR_LABEL);
            IR_t *lfls = ir_alloc(IR_LABEL);
            IR_t *done = ir_alloc(IR_LABEL);
            chain_resolve(&result.tru, ltru);
            chain_resolve(&result.fls, lfls);
            ir_append(&result, ltru);
            ir_append(&result, ir_alloc(IR_ASSIGN, oprd_tar(), lit_alloc(1)));
            ir_append(&result, ir_alloc(IR_GOTO, done));
            ir_append(&result, lfls);
            ir_append(&result, ir_alloc(IR_ASSIGN, oprd_tar(), lit_alloc(0)));
            ir_append(&result, done);
            RETURN(result);
        }
        case OP_NEG: {
            oprd_t  sub_var = var_alloc(NULL, node->super.fst_l);
            ir_list result  = ast_gen(node->sub, sub_var);
            ir_append(&result, ir_alloc(IR_BINARY, OP_SUB, oprd_tar(), lit_alloc(0), sub_var));
            RETURN(result);
        }
        default: UNREACHABLE;
    }
}

VISIT(EXPR_IDEN) {
    ir_list   iden = {0};
    syment_t *sym  = node->sym;
    ASSERT(sym != NULL, "absent sym");

    ir_append(&iden, ir_alloc(IR_ASSIGN, oprd_tar(), sym->var));
    RETURN(iden);
}

VISIT(STMT_RET) {
    oprd_t expr_var = var_alloc(NULL, node->super.fst_l);

    ir_list expr = ast_gen(node->expr, expr_var);
    ir_append(&expr, ir_alloc(IR_RETURN, expr_var));
    RETURN(expr);
}

VISIT(STMT_WHLE) {
    ir_list cond1  = cond_gen(node->cond);
    ir_list cond2  = cond_gen(node->cond);
    ir_list body   = ast_gen(node->body, var_alloc(NULL, node->super.fst_l));
    ir_list result = {0};

    IR_t *pre_hdr = ir_alloc(IR_LABEL);
    IR_t *loop    = ir_alloc(IR_LABEL);
    IR_t *done    = ir_alloc(IR_LABEL);

    chain_resolve(&cond1.tru, pre_hdr);
    chain_resolve(&cond1.fls, done);
    chain_resolve(&cond2.tru, loop);
    chain_resolve(&cond2.fls, done);

    ir_concat(&result, cond1);
    ir_append(&result, pre_hdr);
    ir_append(&result, loop);
    ir_concat(&result, body);
    ir_concat(&result, cond2);
    ir_append(&result, done);
    RETURN(result);
}

VISIT(STMT_IFTE) {
    ir_list cond     = cond_gen(node->cond);
    ir_list tru_stmt = ast_gen(node->tru_stmt, var_alloc(NULL, node->super.fst_l));

    IR_t *ltru = ir_alloc(IR_LABEL);
    IR_t *done = ir_alloc(IR_LABEL);
    chain_resolve(&cond.tru, ltru);

    if (node->fls_stmt != NULL) {
        ir_list fls_stmt = ast_gen(node->fls_stmt, var_alloc(NULL, node->super.fst_l));

        IR_t *lfls = ir_alloc(IR_LABEL);
        chain_resolve(&cond.fls, lfls);
        ir_append(&cond, lfls);
        ir_concat(&cond, fls_stmt);
        ir_append(&cond, ir_alloc(IR_GOTO, done));
    } else {
        chain_resolve(&cond.fls, done);
    }
    ir_append(&cond, ltru);
    ir_concat(&cond, tru_stmt);
    ir_append(&cond, done);
    RETURN(cond);
}

VISIT(STMT_SCOP) {
    ir_list result = {0};
    LIST_ITER(node->decls, it) {
        ir_concat(&result, ast_gen(it, var_alloc(NULL, node->super.fst_l)));
    }
    LIST_ITER(node->stmts, it) {
        ir_concat(&result, ast_gen(it, var_alloc(NULL, node->super.fst_l)));
    }
    RETURN(result);
}

VISIT(EXPR_DOT) {
    oprd_t  pos  = var_alloc(NULL, node->super.fst_l);
    ir_list expr = lexpr_gen((AST_t *) node, pos);
    ir_append(&expr, ir_alloc(IR_LOAD, oprd_tar(), pos));
    RETURN(expr);
}

VISIT(EXPR_ASS) {
    if (node->lhs->kind == EXPR_IDEN) {
        INSTANCE_OF(node->lhs, EXPR_IDEN) {
            ir_list rhs = ast_gen(node->rhs, cnode->sym->var);
            ir_append(&rhs, ir_alloc(IR_ASSIGN, oprd_tar(), cnode->sym->var));
            RETURN(rhs);
        }
    } else {
        oprd_t  lhs_var = var_alloc(NULL, node->super.fst_l);
        oprd_t  rhs_var = var_alloc(NULL, node->super.fst_l);
        ir_list lhs     = lexpr_gen(node->lhs, lhs_var);
        ir_list rhs     = ast_gen(node->rhs, rhs_var);
        ir_concat(&rhs, lhs);
        ir_append(&rhs, ir_alloc(IR_STORE, lhs_var, rhs_var));
        ir_append(&rhs, ir_alloc(IR_ASSIGN, oprd_tar(), rhs_var));
        RETURN(rhs);
    }
    UNREACHABLE;
}

VISIT(CONS_PROG) {
    LIST_ITER(node->decls, it) {
        ast_gen(it, (oprd_t){0});
    }
}

VISIT(CONS_FUN) {
    extern ir_fun_t *prog;
    if (node->body == NULL) {
        RETURN((ir_list){0});
    }

    ir_fun_t *fun   = zalloc(sizeof(ir_fun_t));
    ir_list   param = {0};
    LIST_ITER(node->params, it) {
        INSTANCE_OF(it, DECL_VAR) {
            IR_t *ir = ir_alloc(IR_PARAM, cnode->sym->var);
            ir_append(&param, ir);
        }
    }

    ir_list body = ast_gen(node->body, (oprd_t){0});
    ir_concat(&param, body);
    fun->instrs = param;
    fun->next   = prog;
    symcpy(fun->str, node->str);
    prog = fun;
    RETURN((ir_list){0});
}

VISIT(DECL_VAR) {
    ir_list   decl = {0};
    syment_t *sym  = node->sym;
    oprd_t    var  = sym->var;
    switch (sym->typ.kind) {
        case TYPE_PRIM_INT: {
            if (node->expr != NULL) {
                ir_list expr = ast_gen(node->expr, var);
                ir_concat(&decl, expr);
            }
            RETURN(decl);
        }
        case TYPE_STRUCT:
        case TYPE_ARRAY: {
            oprd_t dummy_var = var_alloc(NULL, node->super.fst_l);
            ir_append(&decl, ir_alloc(IR_DEC, dummy_var, lit_alloc(sym->typ.size)));
            ir_append(&decl, ir_alloc(IR_DREF, var, dummy_var));
            RETURN(decl);
        }
        default: TODO("gen DECL_VAR");
    }
}

VISIT(EXPR_ARR) {
    oprd_t  pos = var_alloc(NULL, node->super.fst_l);
    ir_list arr = lexpr_gen((AST_t *) node, pos);
    if (node->super.type.kind == TYPE_PRIM_INT) {
        ir_append(&arr, ir_alloc(IR_LOAD, oprd_tar(), pos));
    } else {
        ir_append(&arr, ir_alloc(IR_ASSIGN, oprd_tar(), pos));
    }
    RETURN(arr);
}

VISIT(STMT_EXPR) {
    RETURN(ast_gen(node->expr, oprd_tar()));
}

VISIT(EXPR_CALL) {
    ir_list call = {0};
    if (!symcmp(node->str, "read")) {
        ir_append(&call, ir_alloc(IR_READ, oprd_tar()));
    } else if (!symcmp(node->str, "write")) {
        oprd_t arg_var = var_alloc(NULL, node->super.fst_l);
        call           = ast_gen(node->expr, arg_var);
        ir_append(&call, ir_alloc(IR_WRITE, oprd_tar(), arg_var));
    } else {
        ir_list arglist = {0};
        LIST_ITER(node->expr, it) {
            oprd_t  arg_var = var_alloc(NULL, node->super.fst_l);
            ir_list arg     = ast_gen(it, arg_var);
            ir_prepend(&arglist, ir_alloc(IR_ARG, arg_var));
            // reverse order
            ir_concat(&arg, call);
            call = arg;
        }
        ir_concat(&call, arglist);
        ir_append(&call, ir_alloc(IR_CALL, oprd_tar(), node->str));
    }
    RETURN(call);
}

VISIT_EMPTY(CONS_SPEC);
VISIT_EMPTY(DECL_TYP);

VISIT_UNDEF(EXPR_FLT);