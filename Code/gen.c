#include "ast.h"
#include "ir.h"
#include "common.h"
#include "visitor.h"
#include "symtab.h"

#define RET_TYPE ir_list *
#define ARG list
VISITOR_DEF(AST, gen, RET_TYPE);

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

    IR_t *ir = ir_alloc(
        IR_BINARY,
        node->op,
        var_alloc(NULL), lhs_var, rhs_var);
    ir_concat(&lhs, &rhs);
    ir_append(&lhs, ir);
    RETURN(lhs);
}

VISIT(EXPR_UNR) {
    ir_list sub = {0};
    ast_gen(node->sub, &sub);
    oprd_t sub_var = sub.var;

    IR_t *ir = ir_alloc(
        IR_BINARY,
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
    if (node->fls_stmt != NULL) {
        ast_gen(node->fls_stmt, &fls_stmt);
    }
    TODO("gen STMT_IFTE");
}

VISIT(STMT_SCOP) {
    ast_iter(node->decls, it) {
        TODO("gen DECLS");
    }
    ir_list result = {0};
    ast_iter(node->stmts, it) {
        ir_list stmt = {0};
        ast_gen(it, &stmt);
        ir_concat(&result, &stmt);
    }
    RETURN(result);
}

VISIT(EXPR_DOT) {
    TODO("gen EXPR_DOT");
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
    TODO("gen DECL_VAR");
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
    ir_append(&call, ir_alloc(IR_CALL, var_alloc(NULL), node->str));
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
    TODO("gen DECL_TYP");
}