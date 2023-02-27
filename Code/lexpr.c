#include "ast.h"
#include "ir.h"
#include "common.h"
#include "type.h"
#include "visitor.h"
#include "symtab.h"

#define RET_TYPE ir_list *
#define ARG list
VISITOR_DEF(AST, lexpr, RET_TYPE);

void lexpr_gen(AST_t *node, ir_list *list) {
    VISITOR_DISPATCH(AST, lexpr, node, list);
}

VISIT(EXPR_INT) {
    UNREACHABLE;
}

VISIT(EXPR_FLT) {
    UNREACHABLE;
}

VISIT(EXPR_BIN) {
    UNREACHABLE;
}

VISIT(EXPR_UNR) {
    UNREACHABLE;
}

VISIT(EXPR_IDEN) {
    ir_list   iden = {0};
    syment_t *sym  = node->sym;
    ASSERT(sym != NULL, "absent sym");

    ir_append(&iden, ir_alloc(IR_DREF, var_alloc(NULL), sym->var));
    RETURN(iden);
}

VISIT(STMT_RET) {
    UNREACHABLE;
}

VISIT(STMT_WHLE) {
    UNREACHABLE;
}

VISIT(STMT_IFTE) {
    UNREACHABLE;
}

VISIT(STMT_SCOP) {
    UNREACHABLE;
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
    RETURN(base);
}

VISIT(EXPR_ASS) {
    UNREACHABLE;
}

VISIT(CONS_PROG) {
    UNREACHABLE;
}

VISIT(CONS_FUN) {
    UNREACHABLE;
}

VISIT(DECL_VAR) {
    UNREACHABLE;
}

VISIT(EXPR_ARR) {
    TODO("lexpr EXPR_ARR");
}

VISIT(STMT_EXPR) {
    UNREACHABLE;
}

VISIT(EXPR_CALL) {
    UNREACHABLE;
}

VISIT(DECL_FUN) {
    UNREACHABLE;
}

VISIT(CONS_SPEC) {
    UNREACHABLE;
}

VISIT(DECL_TYP) {
    UNREACHABLE;
}