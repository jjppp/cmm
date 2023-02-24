#include "ir.h"
#include "common.h"
#include "visitor.h"
#include <stdarg.h>

const char *IR_NAMES[] = {IR(STRING_LIST) "\0"};

void IR_visitor_dispatch(const struct IR_visitor visitor, IR_t *node, void *p) {
    if (node == NULL) {
        return;
    }
    LOG("%s at %s", visitor.name, IR_NAMES[node->kind]);
#define IR_DISPATCH(NODE)                                             \
    case NODE:                                                        \
        ASSERT(visitor.visit_##NODE != NULL,                          \
               "%s has no method %s", visitor.name, STRINGIFY(NODE)); \
        visitor.visit_##NODE(node, p);                                \
        break;

    switch (node->kind) {
        IR(IR_DISPATCH)
        default: UNREACHABLE;
    }
}

oprd_t var_alloc() {
    static u32 cnt = 0;
    return (oprd_t){
        .kind = OPRD_VAR,
        .val  = cnt++};
}

oprd_t lit_alloc(u32 value) {
    return (oprd_t){
        .kind = OPRD_LIT,
        .val  = value};
}

#define RET_TYPE va_list
#define ARG ap
VISITOR_DEF(IR, new, RET_TYPE);

IR_t *ir_alloc(ir_kind_t kind, ...) {
    va_list ap;
    va_start(ap, kind);

    IR_t *ir = zalloc(sizeof(IR_t));
    ir->kind = kind;
    VISITOR_DISPATCH(IR, new, ir, ap);
    return ir;
}

void ir_append(ir_list *list, IR_t *ir) {
    if (list->size == 0) {
        list->head = list->tail = ir;
        ir->prev = ir->next = NULL;
    } else {
        ir->prev = list->tail;
        ir->next = NULL;

        list->tail->next = ir;
        list->tail       = ir;
    }
    list->var = ir->tar;
    list->size++;
}

void ir_concat(ir_list *front, ir_list *back) {
    if (back->size == 0) {
        return;
    }
    if (front->size == 0) {
        front = back;
        return;
    }
    front->tail->next = back->head;
    back->head->prev  = front->tail;
    front->size += back->size;
    front->var = back->var;
}

VISIT(IR_LABEL) {
    TODO("LABEL");
}

VISIT(IR_FUNCTION) {
    TODO("FUNCTION");
}

VISIT(IR_ASSIGN) {
    node->tar = va_arg(ap, oprd_t);
    node->rhs = va_arg(ap, oprd_t);
}

VISIT(IR_BINARY) {
    // TODO: REL short-circuit
    node->op  = va_arg(ap, op_kind_t);
    node->tar = va_arg(ap, oprd_t);
    node->lhs = va_arg(ap, oprd_t);
    node->rhs = va_arg(ap, oprd_t);
}

VISIT(IR_UNARY) {
    node->op  = va_arg(ap, op_kind_t);
    node->tar = va_arg(ap, oprd_t);
    node->rhs = va_arg(ap, oprd_t);
    switch (node->op) {
        case OP_NEG: {
            node->lhs = lit_alloc(0);
            node->op  = OP_SUB;
            break;
        }
        case OP_NOT: {
            TODO("gen OP_NOT");
        }
        default: UNREACHABLE;
    }
}

VISIT(IR_DREF) {
    TODO("DREF");
}

VISIT(IR_LOAD) {
    TODO("LOAD");
}

VISIT(IR_STORE) {
    TODO("STORE");
}

VISIT(IR_GOTO) {
    TODO("GOTO");
}

VISIT(IR_BRANCH) {
    TODO("BRANCH");
}

VISIT(IR_RETURN) {
    TODO("RETURN");
}

VISIT(IR_DEC) {
    TODO("DEC");
}

VISIT(IR_ARG) {
    TODO("ARG");
}

VISIT(IR_PARAM) {
    TODO("PARAM");
}

VISIT(IR_CALL) {
    TODO("CALL");
}

VISIT(IR_READ) {
    TODO("READ");
}

VISIT(IR_WRITE) {
    TODO("WRITE");
}