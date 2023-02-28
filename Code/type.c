#include "common.h"
#include "symtab.h"
#include "type.h"
#include <string.h>

type_t type_int = (type_t){
    .kind     = TYPE_PRIM_INT,
    .elem_typ = NULL,
    .fields   = NULL,
    .is_ref   = false,
    .size     = 4};

type_t type_flt = (type_t){
    .kind     = TYPE_PRIM_FLT,
    .elem_typ = NULL,
    .fields   = NULL,
    .is_ref   = false,
    .size     = 4};

type_t type_err = (type_t){
    .kind     = TYPE_ERR,
    .elem_typ = NULL,
    .fields   = NULL,
    .is_ref   = false,
    .size     = 0};

field_t *field_alloc(type_t typ, const char str[]) {
    field_t *ptr = zalloc(sizeof(field_t));
    symcpy(ptr->str, str);
    ptr->typ = typ;
    return ptr;
}

void field_free(field_t *field) {
    if (field->next != NULL) {
        field_free(field->next);
    }
    typ_free(field->typ);
    zfree(field);
}

void typ_free(type_t typ) {
    if (typ.is_ref) {
        return;
    }
    if (typ.fields != NULL) {
        field_free(typ.fields);
    }
    if (typ.elem_typ != NULL) {
        typ_free(*typ.elem_typ);
        zfree(typ.elem_typ);
        typ.elem_typ = NULL;
    }
}

static bool field_eq(field_t *f1, field_t *f2) {
    if (f1 == f2) {
        return true;
    }
    if (f1 == NULL || f2 == NULL) {
        return false;
    }
    if (!type_eq(f1->typ, f2->typ)) {
        return false;
    }
    return field_eq(f1->next, f2->next);
}

bool type_eq(type_t typ1, type_t typ2) {
    if (typ1.kind == TYPE_ERR || typ2.kind == TYPE_ERR) {
        return true;
    }
    if (typ1.kind != typ2.kind) {
        return false;
    }
    switch (typ1.kind) {
        case TYPE_PRIM_FLT:
        case TYPE_PRIM_INT:
            return true;
        case TYPE_STRUCT:
            return field_eq(typ1.fields, typ2.fields);
        case TYPE_ARRAY:
            return type_eq(*typ1.elem_typ, *typ2.elem_typ);
        default:
            UNREACHABLE;
    }
    UNREACHABLE;
}

u32 typ_set_size(type_t *typ) {
    u32 size = 0;
    switch (typ->kind) {
        case TYPE_PRIM_FLT: size = 4; break;
        case TYPE_PRIM_INT: size = 4; break;
        case TYPE_STRUCT: {
            LIST_ITER(typ->fields, it) {
                it->off = size;
                size += typ_set_size(&it->typ);
            }
            break;
        }
        case TYPE_ARRAY: {
            size = typ->elem_typ->size;
            for (u32 i = typ->dim; i > 0; i--) {
                typ->acc[i - 1] = size;
                size *= typ->len[i - 1];
            }
            break;
        }
        default: {
            UNREACHABLE;
        }
    }
    return typ->size = size;
}

bool field_exist(field_t *field, const char *str) {
    LIST_ITER(field, it) {
        if (!symcmp(it->str, str)) {
            return true;
        }
    }
    return false;
}