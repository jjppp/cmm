#pragma once

#include "common.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

extern bool sem_err;

/* err ids */
enum err_id {
    ERR_NONE = 0,
    ERR_VAR_UNDEF,
    ERR_FUN_UNDEF,
    ERR_VAR_REDEF,
    ERR_FUN_REDEF,
    ERR_ASS_MISMATCH,
    ERR_ASS_TO_RVALUE,
    ERR_EXP_OPERAND_MISMATCH,
    ERR_RET_MISMATCH,
    ERR_FUN_ARG_MISMATCH,
    ERR_ACC_NON_ARRAY,
    ERR_CALL_NON_FUN,
    ERR_ACC_INDEX,
    ERR_ACC_NON_STRUCT,
    ERR_ACC_UNDEF_FIELD,
    ERR_FIELD_REDEF,
    ERR_STRUCT_REDEF,
    ERR_STRUCT_UNDEF,
    ERR_FUN_DEC_UNDEF,
    ERR_FUN_DEC_COLLISION
};

static const char *SEM_ERR_MSG[] = {
    "NONE",
    "Undefined Variable \"%s\"",
    "Undefined Function \"%s\"",
    "Variable \"%s\" Redefined",
    "Function \"%s\" Redefined",
    "Type mismatched for assignment",
    "The lhs of an assignment must be lvalue",
    "Type mismatched for operands",
    "Type mismatched for return",
    "Function \"%s\" is called with mismatched arguments",
    "\"%s\" is not an array",
    "\"%s\" is not a function",
    "\"%s\" is not an integer",
    "\"%s\" is not a struct",
    "Non-existent field \"%s\"",
    "Redefined field \"%s\"",
    "Duplicated name \"%s\"",
    "Undefined structure \"%s\"",
    "Undefined function \"%s\"",
    "Inconsistent declaration of function \"%s\"",
    "\0"};

#define SEM_ERR(...)                        \
    do {                                    \
        _SEM_ERR(__VA_ARGS__);              \
        RETURN((type_t){.kind = TYPE_ERR}); \
    } while (0)

static inline void _SEM_ERR(enum err_id id, u32 line, ...) {
    va_list ap;
    va_start(ap, line);

    static char buf[8192];
    if (strchr(SEM_ERR_MSG[id], '%') != NULL) {
        snprintf(buf, sizeof(buf), SEM_ERR_MSG[id], va_arg(ap, char *));
    } else {
        snprintf(buf, sizeof(buf), "%s", SEM_ERR_MSG[id]);
    }

    sem_err = true;
    printf("Error type %2d at Line %4d: %s\n", id, line, buf);

    va_end(ap);
}