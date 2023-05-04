#include "common.h"
#include "mips.h"
#include "ir.h"
#include "symtab.h"
#include "visitor.h"
#include <stdarg.h>

const char *REGS_NAMES[] = {REGS(STRING_LIST) "\0"};

#define RET_TYPE va_list
#define ARG p_res
VISITOR_DEF(IR, mips_gen, RET_TYPE);

static ir_fun_t *cur_fun;
static u32       narg;
static FILE     *fout;

static void emit(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(fout, fmt, ap);
    fprintf(fout, "\n");
    va_end(ap);
}

static void mips_gen_fun(ir_fun_t *fun) {
    emit("__fun__%s:", fun->str);
    cur_fun = fun;
    LIST_ITER(fun->instrs.head, it) {
        fprintf(fout, "#");
        ir_print(fout, it);
        VISITOR_DISPATCH(IR, mips_gen, it, NULL);
    }
}

static ir_fun_t *get_fun(const char *str) {
    extern ir_fun_t *prog;

    LIST_ITER(prog, it) {
        if (!symcmp(it->str, str)) {
            return it;
        }
    }
    UNREACHABLE;
}

void mips_gen(FILE *file, ir_fun_t *prog) {
    fout = file;
    LIST_FOREACH(prog, reg_alloc);
    emit(".data\n"
         "_prompt: .asciiz \"Enter an integer:\"\n"
         "_ret: .asciiz \"\\n\"\n"
         ".globl main\n"
         ".text\n"
         "read:\n"
         "  li $v0, 4\n"
         "  la $a0, _prompt\n"
         "  syscall\n"
         "  li $v0, 5\n"
         "  syscall\n"
         "  jr $ra\n"
         "write:\n"
         "  li $v0, 1\n"
         "  syscall\n"
         "  li $v0, 4\n"
         "  la $a0, _ret\n"
         "  syscall\n"
         "  move $v0, $0\n"
         "  jr $ra\n"
         "main:\n"
         "  addi $sp, $sp, -%d\n"
         "  addi $sp, $sp, -4\n"
         "  sw $ra, 0($sp)\n"
         "  jal __fun__main\n"
         "  lw $ra, 0($sp)\n"
         "  addi $sp, $sp, 4\n"
         "  addi $sp, $sp, %d\n"
         "  jr $ra\n",
         get_fun("main")->sf_size,
         get_fun("main")->sf_size);

    LIST_FOREACH(prog, mips_gen_fun);
}

static void load_oprd(const oprd_t *oprd, regs_t reg) {
    switch (oprd->kind) {
        case OPRD_VAR: {
            emit("  lw %s, %d($sp)", REGS_NAMES[reg], cur_fun->sf_size - oprd->offset + 4);
            break;
        }
        case OPRD_LIT: {
            emit("  li %s, %d", REGS_NAMES[reg], oprd->val);
            break;
        }
        default: UNREACHABLE;
    }
}

static void store_oprd(const oprd_t *oprd, regs_t reg) {
    ASSERT(oprd->kind == OPRD_VAR, "storing non var");
    emit("  sw %s, %d($sp)\n", REGS_NAMES[reg], cur_fun->sf_size - oprd->offset + 4);
}

VISIT(IR_LABEL) {
    emit("%s:", node->str);
}

VISIT(IR_ASSIGN) {
    load_oprd(&node->lhs, $t0);
    store_oprd(&node->tar, $t0);
}

VISIT(IR_BINARY) {
    load_oprd(&node->lhs, $t0);
    load_oprd(&node->rhs, $t1);
    const char *op_str = NULL;
    switch (node->op) {
        case OP_ADD: {
            op_str = "add";
            break;
        }
        case OP_SUB: {
            op_str = "sub";
            break;
        }
        case OP_MUL: {
            op_str = "mul";
            break;
        }
        case OP_DIV: {
            op_str = "div";
            break;
        }
        default: UNREACHABLE;
    }
    emit("  %s %s, %s, %s", op_str, REGS_NAMES[$t2], REGS_NAMES[$t0], REGS_NAMES[$t1]);
    store_oprd(&node->tar, $t2);
}

VISIT(IR_DREF) {
    emit("  addi $t0, $sp, %d", cur_fun->sf_size - node->lhs.offset + 4);
    store_oprd(&node->tar, $t0);
}

VISIT(IR_LOAD) {
    load_oprd(&node->lhs, $t0);
    emit("  lw %s, 0(%s)\n", REGS_NAMES[$t1], REGS_NAMES[$t0]);
    store_oprd(&node->tar, $t1);
}

VISIT(IR_STORE) {
    load_oprd(&node->tar, $t0);
    load_oprd(&node->lhs, $t1);
    emit("  sw %s, 0(%s)\n", REGS_NAMES[$t1], REGS_NAMES[$t0]);
}

VISIT(IR_GOTO) {
    emit("  j %s", node->jmpto->str);
}

VISIT(IR_BRANCH) {
    load_oprd(&node->lhs, $t0);
    load_oprd(&node->rhs, $t1);
    const char *op_str = NULL;
    switch (node->op) {
        case OP_EQ: {
            op_str = "beq";
            break;
        }
        case OP_NE: {
            op_str = "bne";
            break;
        }
        case OP_LT: {
            op_str = "blt";
            break;
        }
        case OP_GT: {
            op_str = "bgt";
            break;
        }
        case OP_LE: {
            op_str = "ble";
            break;
        }
        case OP_GE: {
            op_str = "bge";
            break;
        }
        default: UNREACHABLE;
    }
    emit("  %s %s, %s, %s", op_str, REGS_NAMES[$t0], REGS_NAMES[$t1], node->jmpto->str);
}

VISIT(IR_RETURN) {
    load_oprd(&node->lhs, $v0);
    emit("  jr $ra");
}

VISIT(IR_ARG) {
    narg++;
    load_oprd(&node->lhs, $t0);
    emit("  sw $t0, %d($sp)", -narg * 4);
}

VISIT(IR_CALL) {
    // args & locals
    ir_fun_t *tar = get_fun(node->str);
    emit("  addi $sp, $sp, %d", -tar->sf_size);

    emit("  addi $sp, $sp, -4");
    emit("  sw $ra, 0($sp)");
    emit("  jal __fun__%s", node->str);
    emit("  lw $ra, 0($sp)");
    emit("  addi $sp, $sp, 4");

    // locals
    emit("  addi $sp, $sp, %d", tar->sf_size);
    store_oprd(&node->tar, $v0);
    narg = 0;
}

VISIT(IR_READ) {
    emit("  addi $sp, $sp, -4");
    emit("  sw $ra, 0($sp)");
    emit("  jal read");
    emit("  lw $ra, 0($sp)");
    emit("  addi $sp, $sp, 4");
    store_oprd(&node->tar, $v0);
}

VISIT(IR_WRITE) {
    load_oprd(&node->lhs, $a0);
    emit("  addi $sp, $sp, -4");
    emit("  sw $ra, 0($sp)");
    emit("  jal write");
    emit("  lw $ra, 0($sp)");
    emit("  addi $sp, $sp, 4");
}

VISIT_EMPTY(IR_DEC);
VISIT_EMPTY(IR_PARAM);