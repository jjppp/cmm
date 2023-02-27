#include "ast.h"
#include "ir.h"
#include "common.h"
#include "type.h"
#include "visitor.h"
#include "symtab.h"

#define RET_TYPE ir_list *
#define ARG list
VISITOR_DEF(AST, gen, RET_TYPE);

ir_list lexpr_gen(AST_t *node);

ir_list ast_gen(AST_t *node) {
    ir_list ARG;
    VISITOR_DISPATCH(AST, gen, node, &ARG);
    return ARG;
}

VISIT(EXPR_INT) {
    ir_list lit = {0};
    IR_t   *ir  = ir_alloc(
        IR_ASSIGN,
        var_alloc(NULL), lit_alloc(node->value));
    ir_append(&lit, ir);
    RETURN(lit);
}

VISIT(EXPR_BIN) {
    ir_list lhs = ast_gen(node->lhs);
    ir_list rhs = ast_gen(node->rhs);

    oprd_t lhs_var = lhs.var;
    oprd_t rhs_var = rhs.var;
    oprd_t tar_var = var_alloc(NULL);

    switch (node->op) {
        REL_OPS(CASE) {
            IR_t *ltru = ir_alloc(IR_LABEL);
            IR_t *done = ir_alloc(IR_LABEL);
            IR_t *cmp  = ir_alloc(
                IR_BRANCH,
                node->op, lhs_var, rhs_var, ltru);

            done->tar = tar_var; // workaround to make sure lhs.var == tar_var

            ir_concat(&lhs, rhs);
            ir_append(&lhs, cmp);
            ir_append(&lhs, ir_alloc(IR_ASSIGN, tar_var, lit_alloc(0)));
            ir_append(&lhs, ir_alloc(IR_GOTO, done));
            ir_append(&lhs, ltru);
            ir_append(&lhs, ir_alloc(IR_ASSIGN, tar_var, lit_alloc(1)));
            ir_append(&lhs, done);
            break;
        }
        LOGIC_OPS(CASE) {
            /*  if (lhs != 0) goto ltru
                    <false stuff>
                    goto done
                ltru:
                    <true stuff>
                done:
                    <finally> */
            IR_t *ltru = ir_alloc(IR_LABEL);
            IR_t *done = ir_alloc(IR_LABEL);
            IR_t *cmp  = ir_alloc(
                IR_BRANCH,
                OP_NE, lhs_var, lit_alloc(0), ltru);

            done->lhs = tar_var; // workaround to make sure lhs.var == tar_var

            ir_append(&lhs, cmp);
            switch (node->op) {
                case OP_AND: {
                    ir_append(&lhs, ir_alloc(IR_ASSIGN, tar_var, lit_alloc(0)));
                    ir_append(&lhs, ir_alloc(IR_GOTO, done));
                    ir_append(&lhs, ltru);
                    ir_concat(&lhs, rhs);
                    ir_append(&lhs, ir_alloc(IR_ASSIGN, tar_var, rhs_var));
                    ir_append(&lhs, done);
                    break;
                }
                case OP_OR: {
                    ir_concat(&lhs, rhs);
                    ir_append(&lhs, ir_alloc(IR_ASSIGN, tar_var, rhs_var));
                    ir_append(&lhs, ir_alloc(IR_GOTO, done));
                    ir_append(&lhs, ltru);
                    ir_append(&lhs, ir_alloc(IR_ASSIGN, tar_var, lit_alloc(1)));
                    ir_append(&lhs, done);
                    break;
                }
                default: TODO("gen BINARY LOGIC_OPS");
            }
            break;
        }
        ARITH_OPS(CASE) {
            IR_t *ir = ir_alloc(
                IR_BINARY,
                node->op,
                tar_var, lhs_var, rhs_var);
            ir_concat(&lhs, rhs);
            ir_append(&lhs, ir);
            break;
        }
        default: UNREACHABLE;
    }
    RETURN(lhs);
}

VISIT(EXPR_UNR) {
    ir_list sub     = ast_gen(node->sub);
    oprd_t  sub_var = sub.var;

    IR_t *ir = ir_alloc(
        IR_UNARY,
        node->op,
        var_alloc(NULL), sub_var);
    ir_append(&sub, ir);
    RETURN(sub);
}

VISIT(EXPR_IDEN) {
    ir_list   iden = {0};
    syment_t *sym  = node->sym;
    ASSERT(sym != NULL, "absent sym");

    ir_append(&iden, ir_alloc(IR_ASSIGN, var_alloc(NULL), sym->var));
    RETURN(iden);
}

VISIT(STMT_RET) {
    ir_list expr     = ast_gen(node->expr);
    oprd_t  expr_var = expr.var;

    IR_t *ir = ir_alloc(
        IR_RETURN,
        expr_var);
    ir_append(&expr, ir);
    RETURN(expr);
}

VISIT(STMT_WHLE) {
    /* while (cond) body =>
       if (!cond1) goto done
         loop:
         <body>
         if (cond2) goto loop
       done:
    */
    ir_list cond1 = ast_gen(node->cond);
    ir_list cond2 = ast_gen(node->cond);
    ir_list body  = ast_gen(node->body);
    IR_t   *loop  = ir_alloc(IR_LABEL);
    IR_t   *done  = ir_alloc(IR_LABEL);

    ir_append(&cond1, ir_alloc(IR_BRANCH, OP_EQ, cond1.var, lit_alloc(0), done));

    ir_prepend(&body, loop);
    ir_concat(&body, cond2);
    ir_append(&body, ir_alloc(IR_BRANCH, OP_NE, cond2.var, lit_alloc(0), loop));
    ir_append(&body, done);
    RETURN(body);
}

VISIT(STMT_IFTE) {
    ir_list cond     = ast_gen(node->cond);
    ir_list tru_stmt = ast_gen(node->tru_stmt);

    IR_t *ltru = ir_alloc(IR_LABEL);
    IR_t *done = ir_alloc(IR_LABEL);

    oprd_t cond_var = cond.var;
    done->lhs       = cond_var; // workaround to make sure lhs.var == tar_var

    ir_append(
        &cond,
        ir_alloc(
            IR_BRANCH,
            OP_NE, cond_var, lit_alloc(0), ltru));
    if (node->fls_stmt != NULL) {
        ir_list fls_stmt = ast_gen(node->fls_stmt);
        ir_concat(&cond, fls_stmt);
        ir_append(&cond, ir_alloc(IR_GOTO, done));
    }
    ir_append(&cond, ltru);
    ir_concat(&cond, tru_stmt);
    ir_append(&cond, done);
    RETURN(cond);
}

VISIT(STMT_SCOP) {
    ir_list result = {0};
    ast_iter(node->decls, it) {
        ir_list decl = ast_gen(it);
        ir_concat(&result, decl);
    }
    ast_iter(node->stmts, it) {
        ir_list stmt = ast_gen(it);
        ir_concat(&result, stmt);
    }
    RETURN(result);
}

VISIT(EXPR_DOT) {
    ir_list expr = lexpr_gen((AST_t *) node);
    oprd_t  pos  = expr.var;
    ir_append(&expr, ir_alloc(IR_LOAD, var_alloc(NULL), pos));
    RETURN(expr);
}

VISIT(EXPR_ASS) {
    ir_list rhs     = ast_gen(node->rhs);
    oprd_t  rhs_var = rhs.var;

    if (node->lhs->kind == EXPR_IDEN) {
        INSTANCE_OF(node->lhs, EXPR_IDEN) {
            oprd_t tar_var = cnode->sym->var;
            IR_t  *ir      = ir_alloc(
                IR_ASSIGN,
                tar_var, rhs_var);
            ir_append(&rhs, ir);
        }
    } else {
        ir_list lhs     = lexpr_gen(node->lhs);
        oprd_t  lhs_var = lhs.var;

        ir_concat(&rhs, lhs);
        ir_append(&rhs, ir_alloc(IR_STORE, lhs_var, rhs_var));
    }
    RETURN(rhs);
}

VISIT(CONS_PROG) {
    ir_list global_decls = {0};
    ast_iter(node->decls, it) {
        ir_concat(&global_decls, ast_gen(it));
    }
    RETURN(global_decls);
}

VISIT(CONS_FUN) {
    extern ir_fun_t *prog;
    if (node->body == NULL) {
        RETURN((ir_list){0});
    }

    ir_fun_t *fun   = zalloc(sizeof(ir_fun_t));
    ir_list   param = {0};
    ast_iter(node->params, it) {
        INSTANCE_OF(it, DECL_VAR) {
            IR_t *ir = ir_alloc(IR_PARAM, cnode->sym->var);
            ir_append(&param, ir);
        }
    }

    ir_list body = ast_gen(node->body);
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
                ir_list expr = ast_gen(node->expr);

                IR_t *ir = ir_alloc(IR_ASSIGN, var, expr.var);
                ir_concat(&decl, expr);
                ir_append(&decl, ir);
            }
            RETURN(decl);
        }
        case TYPE_STRUCT:
        case TYPE_ARRAY: {
            oprd_t dummy_var = var_alloc(NULL);
            ir_append(&decl, ir_alloc(IR_DEC, dummy_var, lit_alloc(sym->typ.size)));
            ir_append(&decl, ir_alloc(IR_DREF, var, dummy_var));
            RETURN(decl);
        }
        default: TODO("gen DECL_VAR");
    }
}

VISIT(EXPR_ARR) {
    ir_list arr = lexpr_gen((AST_t *) node);
    oprd_t  pos = arr.var;
    ir_append(&arr, ir_alloc(IR_LOAD, var_alloc(NULL), pos));
    RETURN(arr);
}

VISIT(STMT_EXPR) {
    RETURN(ast_gen(node->expr));
}

VISIT(EXPR_CALL) {
    ir_list call = {0};
    ast_iter(node->expr, it) {
        ir_list arg = ast_gen(it);
        ir_append(&arg, ir_alloc(IR_ARG, arg.var));
        // reverse order
        ir_concat(&arg, call);
        call = arg;
    }
    oprd_t tar_var = var_alloc(NULL);
    if (!symcmp(node->str, "read")) {
        ir_append(&call, ir_alloc(IR_READ, tar_var));
    } else if (!symcmp(node->str, "write")) {
        oprd_t arg_var = call.var;
        ir_append(&call, ir_alloc(IR_WRITE, tar_var, arg_var));
    } else {
        ir_append(&call, ir_alloc(IR_CALL, tar_var, node->str));
    }
    RETURN(call);
}

VISIT_EMPTY(CONS_SPEC);
VISIT_EMPTY(DECL_TYP);

VISIT_UNDEF(EXPR_FLT);