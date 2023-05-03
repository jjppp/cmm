#include "hashtab.h"
#include "ir.h"
#include "visitor.h"
#include "mips.h"
#include "symtab.h"
#include <string.h>

/**
 * This dummy reg-alloc algorithm simply put all
 * OPRD_VAR on the stack, and generate corresponding
 * offsets for future use.
 */

static hashtab_t hashtab;
static uptr      offset;

#define RET_TYPE va_list
#define ARG p_res
VISITOR_DEF(IR, mips_reg, RET_TYPE);

/**
 * Stack Layout:
 *        ____________
 * fp -> |   ...      |
 *       |   params   |
 *       |   ...      |
 *       |   locals   |
 *       |   ...      |
 * sp -> |   ret_addr |
 *        ------------
 */

// alloc 4 bytes for non-param-regs if not previously allocated
static void alloc(oprd_t *oprd) {
    if (oprd->kind != OPRD_VAR) {
        return;
    }
    hashent_t *ent = hash_lookup(&hashtab, oprd->name);
    if (ent->ptr == NULL) {
        offset += 4;
        symcpy(ent->str, oprd->name);
        ent->ptr     = (void *) offset;
        oprd->offset = offset;
    } else {
        oprd->offset = (uptr) ent->ptr;
    }
}

void reg_alloc(ir_fun_t *fun) {
    memset(&hashtab, 0, sizeof(hashtab));
    offset = 0;

    LIST_ITER(fun->instrs.head, it) {
        if (it->kind == IR_PARAM) {
            alloc(&it->tar);
        }
    }
    LIST_ITER(fun->instrs.head, it) {
        VISITOR_DISPATCH(IR, mips_reg, it, NULL);
    }
    fun->sf_size = offset;
}

VISIT_EMPTY(IR_LABEL);
VISIT_EMPTY(IR_GOTO);
VISIT_EMPTY(IR_PARAM);

VISIT(IR_ASSIGN) {
    alloc(&node->tar);
    alloc(&node->lhs);
}

VISIT(IR_BINARY) {
    alloc(&node->tar);
    alloc(&node->lhs);
    alloc(&node->rhs);
}

VISIT(IR_DREF) {
    alloc(&node->tar);
    alloc(&node->lhs);
}

VISIT(IR_LOAD) {
    alloc(&node->tar);
    alloc(&node->lhs);
}

VISIT(IR_STORE) {
    alloc(&node->tar);
    alloc(&node->lhs);
}

VISIT(IR_BRANCH) {
    alloc(&node->lhs);
    alloc(&node->rhs);
}

VISIT(IR_RETURN) {
    alloc(&node->lhs);
}

VISIT(IR_DEC) {
    // ! Check if multiple `DEC`s can be called with identical lhs regs.
    oprd_t    *oprd = &node->tar;
    hashent_t *ent  = hash_lookup(&hashtab, oprd->name);
    if (ent->ptr == NULL) {
        symcpy(ent->str, oprd->name);
        ent->ptr     = (void *) offset;
        oprd->offset = offset;
        offset += node->lhs.val;
    }
}

VISIT(IR_ARG) {
    alloc(&node->lhs);
}

VISIT(IR_CALL) {
    alloc(&node->tar);
}

VISIT(IR_READ) {
    alloc(&node->tar);
}

VISIT(IR_WRITE) {
    alloc(&node->lhs);
}