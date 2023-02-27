#include "ast.h"
#include "ir.h"
#include "common.h"
#include "type.h"
#include "visitor.h"
#include "symtab.h"

#define RET_TYPE ir_list *
#define ARG list
VISITOR_DEF(AST, gen, RET_TYPE);

void lexpr_gen(AST_t *node, ir_list *list);

void ast_gen(AST_t *node, ir_list *list) {
    VISITOR_DISPATCH(AST, gen, node, list);
}

VISIT(EXPR_INT) {
    ir_list lit = {0};
    IR_t   *ir  = ir_alloc(
        IR_ASSIGN,
        var_alloc(NULL), lit_alloc(node->value));
    ir_append(&lit, ir);
    RETURN(lit);
}

VISIT(EXPR_FLT) {
    TODO("gen EXPR_FLT");
}

VISIT(EXPR_BIN) {
    ir_list lhs = {0}, rhs = {0};
    ast_gen(node->lhs, &lhs);
    ast_gen(node->rhs, &rhs);
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

            ir_concat(&lhs, &rhs);
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
                    ir_concat(&lhs, &rhs);
                    ir_append(&lhs, ir_alloc(IR_ASSIGN, tar_var, rhs_var));
                    ir_append(&lhs, done);
                    break;
                }
                case OP_OR: {
                    ir_concat(&lhs, &rhs);
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
            ir_concat(&lhs, &rhs);
            ir_append(&lhs, ir);
            break;
        }
        default: UNREACHABLE;
    }
    RETURN(lhs);
}

VISIT(EXPR_UNR) {
    ir_list sub = {0};
    ast_gen(node->sub, &sub);
    oprd_t sub_var = sub.var;

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
    ir_list expr = {0};
    ast_gen(node->expr, &expr);
    oprd_t expr_var = expr.var;

    IR_t *ir = ir_alloc(
        IR_RETURN,
        expr_var);
    ir_append(&expr, ir);
    RETURN(expr);
}

VISIT(STMT_WHLE) {
    ir_list cond = {0}, body = {0};
    ast_gen(node->cond, &cond);
    ast_gen(node->body, &body);
    TODO("gen STMT_WHLE");
}

VISIT(STMT_IFTE) {
    ir_list cond = {0}, tru_stmt = {0}, fls_stmt = {0};
    ast_gen(node->cond, &cond);
    ast_gen(node->tru_stmt, &tru_stmt);

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
        ast_gen(node->fls_stmt, &fls_stmt);
        ir_concat(&cond, &fls_stmt);
        ir_append(&cond, ir_alloc(IR_GOTO, done));
    }
    ir_append(&cond, ltru);
    ir_concat(&cond, &tru_stmt);
    ir_append(&cond, done);
    RETURN(cond);
}

VISIT(STMT_SCOP) {
    ir_list result = {0};
    ast_iter(node->decls, it) {
        ir_list decl = {0};
        ast_gen(it, &decl);
        ir_concat(&result, &decl);
    }
    ast_iter(node->stmts, it) {
        ir_list stmt = {0};
        ast_gen(it, &stmt);
        ir_concat(&result, &stmt);
    }
    RETURN(result);
}

VISIT(EXPR_DOT) {
    ir_list base = {0};
    oprd_t  off  = lit_alloc(node->field->off);

    lexpr_gen(node->base, &base);
    oprd_t base_var  = base.var;
    oprd_t field_var = var_alloc(NULL);
    ir_append(
        &base,
        ir_alloc(IR_BINARY,
                 OP_ADD, field_var, base_var, off));
    ir_append(&base, ir_alloc(IR_LOAD, var_alloc(NULL), field_var));
    RETURN(base);
}

VISIT(EXPR_ASS) {
    ir_list rhs = {0};
    ast_gen(node->rhs, &rhs);
    oprd_t rhs_var = rhs.var;

    if (node->lhs->kind == EXPR_IDEN) {
        INSTANCE_OF(node->lhs, EXPR_IDEN) {
            oprd_t tar_var = cnode->sym->var;
            IR_t  *ir      = ir_alloc(
                IR_ASSIGN,
                tar_var, rhs_var);
            ir_append(&rhs, ir);
            RETURN(rhs);
        }
    } else {
        ir_list lhs = {0};
        lexpr_gen(node->lhs, &lhs);
        oprd_t lhs_var = lhs.var;

        ir_concat(&rhs, &lhs);
        ir_append(&rhs, ir_alloc(IR_STORE, lhs_var, rhs_var));
        RETURN(rhs);
    }
    TODO("EXPR_ASS lhs");
}

VISIT(CONS_PROG) {
    ast_iter(node->decls, it) {
        ast_gen(it, NULL);
    }
}

VISIT(CONS_FUN) {
    extern ir_fun_t *prog;

    ir_fun_t *fun   = zalloc(sizeof(ir_fun_t));
    ir_list   param = {0};
    ir_list   body  = {0};

    ast_iter(node->params, it) {
        INSTANCE_OF(it, DECL_VAR) {
            IR_t *ir = ir_alloc(IR_PARAM, cnode->sym->var);
            ir_append(&param, ir);
        }
    }
    ast_gen(node->body, &body);
    ir_concat(&param, &body);
    fun->instrs = param;
    fun->next   = prog;
    symcpy(fun->str, node->str);
    prog = fun;
}

VISIT(DECL_VAR) {
    ir_list   decl = {0};
    syment_t *sym  = node->sym;
    oprd_t    var  = sym->var;
    switch (sym->typ.kind) {
        case TYPE_PRIM_INT: {
            if (node->expr != NULL) {
                ir_list expr = {0};
                ast_gen(node->expr, &expr);

                IR_t *ir = ir_alloc(IR_ASSIGN, var, expr.var);
                ir_concat(&decl, &expr);
                ir_append(&decl, ir);
            }
            RETURN(decl);
        }
        case TYPE_STRUCT: {
            ir_append(&decl, ir_alloc(IR_DEC, var, lit_alloc(sym->typ.size)));
            RETURN(decl);
        }
        default: TODO("gen DECL_VAR");
    }
}

VISIT(EXPR_ARR) {
    TODO("gen EXPR_ARR");
}

VISIT(STMT_EXPR) {
    ir_list expr = {0};
    ast_gen(node->expr, &expr);
    RETURN(expr);
}

VISIT(EXPR_CALL) {
    ir_list call = {0};
    ast_iter(node->expr, it) {
        ir_list arg = {0};
        ast_gen(it, &arg);
        ir_append(&arg, ir_alloc(IR_ARG, arg.var));
        // reverse order
        ir_concat(&arg, &call);
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

VISIT(DECL_FUN) {
    extern ir_fun_t *prog;
    TODO("gen DECL_FUN");
}

VISIT(CONS_SPEC) {
    TODO("gen CONS_SPEC");
}

VISIT(DECL_TYP) {
    // do nothing
}