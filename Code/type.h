#pragma once

#include "common.h"
#include <stdarg.h>
#include <stdbool.h>

typedef struct type_t type_t;

typedef struct field_t field_t;

struct type_t {
    enum type_kind {
        TYPE_NULL = 0,
        TYPE_ERR,
        TYPE_PRIM_INT,
        TYPE_PRIM_FLT,
        TYPE_STRUCT,
        TYPE_ARRAY
    } kind;
    char     str[MAX_SYM_LEN];
    field_t *fields;
    type_t  *elem_typ;
    u32      len[MAX_DIM], dim;
    bool     is_ref; // e.g. struct STRUCT_NAME field;
};

struct field_t {
    field_t *next;
    char     str[MAX_SYM_LEN];
    type_t   typ;
};

#define IS_SCALAR(TYPE)                \
    (((TYPE).kind == TYPE_PRIM_INT)    \
     || ((TYPE).kind == TYPE_PRIM_FLT) \
     || ((TYPE).kind == TYPE_ERR))

#define IS_LOGIC(TYPE)               \
    ((((TYPE).kind == TYPE_PRIM_INT) \
      && IS_SCALAR(TYPE))            \
     || ((TYPE).kind == TYPE_ERR))

#define field_iter(FIELD, IT) \
    for (field_t *IT = (FIELD); (IT) != NULL; (IT) = (IT)->next)

field_t *field_alloc(type_t typ, const char str[]);

void field_free(field_t *field);

void typ_free(type_t typ);

bool type_eq(type_t typ1, type_t typ2);