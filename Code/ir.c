#include "ir.h"
#include "ast.h"
#include "common.h"
#include "visitor.h"
#include "symtab.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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

oprd_t var_alloc(const char *name, u32 lineno) {
    static u32 cnt = 0;
    return (oprd_t){
        .kind   = OPRD_VAR,
        .name   = name,
        .lineno = lineno,
        .val    = ++cnt};
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
                snprintf(buf, sizeof(buf), "t_%u_at_%u_", oprd.val, oprd.lineno);
            }
            break;
        default: UNREACHABLE;
    }
    return buf;
}

oprd_t lit_alloc(u32 value) {
    return (oprd_t){
        .kind = OPRD_LIT,
        .val  = value};
}

void chain_merge(chain_t **into, chain_t *rhs) {
    LIST_APPEND(*into, rhs);
}

void chain_insert(chain_t **chain, IR_t *ir) {
    chain_t *node = zalloc(sizeof(chain_t));

    *node = (chain_t){.ir = ir};
    chain_merge(chain, node);
}

void chain_resolve(chain_t **chain, IR_t *ir) {
    LIST_ITER(*chain, it) {
        it->ir->jmpto = ir;
    }
    *chain = NULL; // TODO: mem leak
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
    ir_validate(list);
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
    ir_validate(list);
}

void ir_prepend(ir_list *list, IR_t *ir) {
    ir_validate(list);
    if (list->size == 0) {
        list->head = ir;
        list->tail = ir;
        ir->prev   = NULL;
        ir->next   = NULL;
        list->var  = ir->tar;
    } else {
        ir->next = list->head;
        ir->prev = NULL;

        list->head->prev = ir;
        list->head       = ir;
    }
    list->size++;
    ir_validate(list);
}

void ir_concat(ir_list *front, const ir_list back) {
    ir_validate(front);
    ir_validate(&back);
    if (back.size == 0) {
        return;
    }
    if (front->size == 0) {
        front->size = back.size;
        front->head = back.head;
        front->tail = back.tail;
        front->var  = back.var;
        return;
    }
    front->tail->next = back.head;
    back.head->prev   = front->tail;
    front->size += back.size;

    front->tail = back.tail;
    front->var  = back.var;
    ir_validate(front);
}

ir_list ir_split(ir_list *list, IR_t *it) {
    ir_validate(list);
    ir_list front = {0};
    ASSERT(list != NULL, "split NULL list");
    ASSERT(it != NULL, "split NULL it");

    if (list->head != it) {
        front = (ir_list){
            .head = it,
            .tail = list->tail,
            .var  = list->var,
            .size = LIST_LENGTH(it)};
        list->tail = it->prev;
        list->size -= front.size;

        it->prev->next = NULL;
        it->prev       = NULL;
        swap(front, *list);
    }
    ir_validate(list);
    ir_validate(&front);
    return front;
}

void ir_validate(const ir_list *list) {
    ASSERT(LIST_LENGTH(list->head) == list->size, "%u != %u", LIST_LENGTH(list->head), list->size);
    IR_t *last = NULL;
    LIST_ITER(list->head, it) {
        last = it;
    }
    ASSERT(last == list->tail, "tail unreachable");
    last = NULL;
    LIST_REV_ITER(list->tail, it) {
        last = it;
    }
    ASSERT(last == list->head, "head unreachable");
}

void ir_remove_mark(ir_list *list) {
    ir_validate(list);
    LIST_ITER(list->head, it) {
        while (it->next && it->next->mark) {
            // void *ptr = it->next;
            if (it->next->next) {
                it->next->next->prev = it;
            }
            it->next = it->next->next;
            // zfree(ptr);
            list->size--;
        }
    }
    if (list->head && list->head->mark) {
        // void *ptr = list->head;
        if (list->head->next) {
            list->head->next->prev = NULL;
        }
        list->head = list->head->next;
        // zfree(ptr);
        list->size--;
    }
    for (list->tail = list->head; list->tail && list->tail->next;) {
        list->tail = list->tail->next;
    }
    ir_validate(list);
}

void ir_check(ir_list *list) {
    LIST_ITER(list->head, it) {
        if (it->kind == IR_NULL) {
            abort();
        } else if ((it->kind == IR_GOTO || it->kind == IR_BRANCH) && it->jmpto == NULL) {
            abort();
        }
    }
}

VISIT(IR_LABEL) {
    snprintf(node->str, sizeof(node->str), "label%u", node->id);
}

VISIT(IR_ASSIGN) {
    node->tar = va_arg(ap, oprd_t);
    node->lhs = va_arg(ap, oprd_t);
}

VISIT(IR_BINARY) {
    node->op  = va_arg(ap, op_kind_t);
    node->tar = va_arg(ap, oprd_t);
    node->lhs = va_arg(ap, oprd_t);
    node->rhs = va_arg(ap, oprd_t);
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

VISIT_UNDEF(IR_NULL);