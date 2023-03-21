#include "ast.h"
#include "ir.h"
#include "common.h"
#include "type.h"
#include "visitor.h"
#include "symtab.h"

#define RET_TYPE ir_list *
#define ARG list
VISITOR_DEF(AST, cond, RET_TYPE);

ir_list cond_gen(AST_t *node) {
    ir_list ARG = {0};
    VISITOR_DISPATCH(AST, cond, node, &ARG);
    return ARG;
}

static ir_list wrapper(AST_t *node) {
    oprd_t result_var = var_alloc(NULL, node->fst_l);

    ir_list result  = ast_gen((AST_t *) node, result_var);
    IR_t   *cmp_tru = ir_alloc(IR_BRANCH, OP_NE, result_var, lit_alloc(0), NULL);
    IR_t   *jmp_fls = ir_alloc(IR_GOTO, NULL);
    ir_append(&result, cmp_tru);
    ir_append(&result, jmp_fls);
    chain_insert(&result.tru, cmp_tru);
    chain_insert(&result.fls, jmp_fls);
    return result;
}

VISIT(EXPR_IDEN) {
    RETURN(wrapper((AST_t *) node));
}

VISIT(EXPR_DOT) {
    RETURN(wrapper((AST_t *) node));
}

VISIT(EXPR_ARR) {
    RETURN(wrapper((AST_t *) node));
}

VISIT(EXPR_CALL) {
    RETURN(wrapper((AST_t *) node));
}

VISIT(EXPR_ASS) {
    RETURN(wrapper((AST_t *) node));
}

VISIT(EXPR_INT) {
    RETURN(wrapper((AST_t *) node));
}

VISIT(EXPR_BIN) { // yields jmp
    ir_list lhs    = {0};
    ir_list rhs    = {0};
    ir_list result = {0};

    oprd_t lhs_var = var_alloc(NULL, node->super.fst_l);
    oprd_t rhs_var = var_alloc(NULL, node->super.fst_l);
    switch (node->op) {
        REL_OPS(CASE) {
            /*
             lhs
             rhs
             cmp(op, lhs, rhs) -> tru
             goto -> fls
            */
            lhs = ast_gen(node->lhs, lhs_var);
            rhs = ast_gen(node->rhs, rhs_var);

            IR_t *cmp_tru = ir_alloc(IR_BRANCH, node->op, lhs_var, rhs_var, NULL);
            IR_t *jmp_fls = ir_alloc(IR_GOTO, NULL);
            ir_concat(&result, lhs);
            ir_concat(&result, rhs);
            ir_append(&result, cmp_tru);
            ir_append(&result, jmp_fls);
            chain_insert(&result.tru, cmp_tru);
            chain_insert(&result.fls, jmp_fls);
            break;
        }
        LOGIC_OPS(CASE) {
            lhs = cond_gen(node->lhs);
            rhs = cond_gen(node->rhs);
            switch (node->op) {
                case OP_AND: {
                    /*
                     lhs
                     tru:
                     rhs
                    */
                    IR_t *tru = ir_alloc(IR_LABEL);
                    chain_resolve(&lhs.tru, tru);
                    ir_concat(&result, lhs);
                    ir_append(&result, tru);
                    ir_concat(&result, rhs);
                    chain_merge(&result.fls, rhs.fls);
                    chain_merge(&result.fls, lhs.fls);
                    chain_merge(&result.tru, rhs.tru);
                    break;
                }
                case OP_OR: {
                    /*
                     lhs
                     fls:
                     rhs
                    */
                    IR_t *fls = ir_alloc(IR_LABEL);
                    chain_resolve(&lhs.fls, fls);
                    ir_concat(&result, lhs);
                    ir_append(&result, fls);
                    ir_concat(&result, rhs);
                    chain_merge(&result.tru, rhs.tru);
                    chain_merge(&result.tru, lhs.tru);
                    chain_merge(&result.fls, rhs.fls);
                    break;
                }
                default: UNREACHABLE;
            }
            break;
        }
        ARITH_OPS(CASE) {
            result = wrapper((AST_t *) node);
            break;
        }
        default: UNREACHABLE;
    }
    RETURN(result);
}

VISIT(EXPR_UNR) {
    ir_list result = {0};
    switch (node->op) {
        case OP_NOT: {
            ir_list sub = cond_gen(node->sub);
            ir_concat(&result, sub);
            chain_merge(&result.fls, sub.tru);
            chain_merge(&result.tru, sub.fls);
            break;
        }
        case OP_NEG: {
            result = wrapper((AST_t *) node);
            break;
        }
        default: UNREACHABLE;
    }
    RETURN(result);
}

VISIT_UNDEF(EXPR_FLT);

VISIT_UNDEF(CONS_PROG);
VISIT_UNDEF(CONS_SPEC);
VISIT_UNDEF(CONS_FUN);

VISIT_UNDEF(DECL_TYP);
VISIT_UNDEF(DECL_VAR);

VISIT_UNDEF(STMT_EXPR);
VISIT_UNDEF(STMT_WHLE);
VISIT_UNDEF(STMT_IFTE);
VISIT_UNDEF(STMT_SCOP);
VISIT_UNDEF(STMT_RET);