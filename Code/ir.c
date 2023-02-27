#include "ir.h"
#include "ast.h"
#include "common.h"
#include "visitor.h"
#include "symtab.h"
#include <stdarg.h>
#include <stdio.h>

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

oprd_t var_alloc(const char *name) {
    static u32 cnt = 0;
    return (oprd_t){
        .kind = OPRD_VAR,
        .name = name,
        .val  = ++cnt};
}

char *oprd_to_str(oprd_t oprd) {
    static char buf[BUFSIZ];
    switch (oprd.kind) {
        case OPRD_LIT:
            snprintf(buf, sizeof(buf), "#%d", oprd.val);
            break;
        case OPRD_VAR:
            if (oprd.name != NULL) {
                snprintf(buf, sizeof(buf), "%s%u", oprd.name, oprd.val);
            } else {
                snprintf(buf, sizeof(buf), "_%u", oprd.val);
            }
            break;
        case OPRD_PTR:
            TODO("OPRD_PTR");
    }
    return buf;
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
    static u32 cnt = 0;

    va_list ap;
    va_start(ap, kind);

    IR_t *ir = zalloc(sizeof(IR_t));
    ir->kind = kind;
    ir->id   = ++cnt;
    VISITOR_DISPATCH(IR, new, ir, ap);

    va_end(ap);
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
        *front = *back;
        return;
    }
    front->tail->next = back->head;
    back->head->prev  = front->tail;
    front->size += back->size;

    front->tail = back->tail;
    front->var  = back->var;
}

VISIT(IR_LABEL) {
    snprintf(node->str, sizeof(node->str), "label%u", node->id);
}

VISIT(IR_ASSIGN) {
    node->tar = va_arg(ap, oprd_t);
    node->lhs = va_arg(ap, oprd_t);
}

VISIT(IR_BINARY) {
    // TODO: REL short-circuit
    node->op  = va_arg(ap, op_kind_t);
    node->tar = va_arg(ap, oprd_t);
    node->lhs = va_arg(ap, oprd_t);
    node->rhs = va_arg(ap, oprd_t);
}

VISIT(IR_UNARY) {
    node->kind = IR_BINARY;
    node->op   = va_arg(ap, op_kind_t);
    node->tar  = va_arg(ap, oprd_t);
    node->rhs  = va_arg(ap, oprd_t);
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
    node->tar = va_arg(ap, oprd_t);
    node->lhs = va_arg(ap, oprd_t);
}

VISIT(IR_LOAD) {
    node->tar = va_arg(ap, oprd_t);
    node->lhs = va_arg(ap, oprd_t);
}

VISIT(IR_STORE) {
    node->tar = va_arg(ap, oprd_t);
    node->lhs = va_arg(ap, oprd_t);
}

VISIT(IR_GOTO) {
    node->jmpto = va_arg(ap, IR_t *);
}

VISIT(IR_BRANCH) {
    node->op    = va_arg(ap, op_kind_t);
    node->lhs   = va_arg(ap, oprd_t);
    node->rhs   = va_arg(ap, oprd_t);
    node->jmpto = va_arg(ap, IR_t *);
}

VISIT(IR_RETURN) {
    node->lhs = va_arg(ap, oprd_t);
}

VISIT(IR_DEC) {
    node->tar = va_arg(ap, oprd_t);
    node->lhs = va_arg(ap, oprd_t);
}

VISIT(IR_ARG) {
    node->lhs = va_arg(ap, oprd_t);
}

VISIT(IR_PARAM) {
    node->lhs = va_arg(ap, oprd_t);
}

VISIT(IR_CALL) {
    node->tar = va_arg(ap, oprd_t);
    symcpy(node->str, va_arg(ap, char *));
}

VISIT(IR_READ) {
    node->tar = va_arg(ap, oprd_t);
}

VISIT(IR_WRITE) {
    node->tar = va_arg(ap, oprd_t);
    node->lhs = va_arg(ap, oprd_t);
}