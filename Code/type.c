#include "common.h"
#include "symtab.h"
#include "type.h"

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

type_t *typ_alloc(enum type_kind kind, ...) {
    va_list ap;
    va_start(ap, kind);

    type_t *ptr = zalloc(sizeof(type_t));
    ptr->kind   = kind;

    switch (kind) {
        case TYPE_PRIM_INT:
        case TYPE_PRIM_FLT:
            break;
        case TYPE_STRUCT:
            symcpy(ptr->str, va_arg(ap, char *));
            ptr->fields = va_arg(ap, field_t *);
            break;
        case TYPE_ARRAY:
            TODO("TYPE_ARRAY");
        default: UNREACHABLE;
    }

    va_end(ap);
    return ptr;
}

void typ_free(type_t typ) {
    if (typ.kind == TYPE_STRUCT && typ.fields != NULL) {
        field_free(typ.fields);
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
            if (typ1.dim != typ2.dim) {
                return false;
            }
            for (u32 i = 0; i < typ1.dim; i++) {
                if (typ1.len[i] != typ2.len[i]) {
                    return false;
                }
            }
            return type_eq(*typ1.elem_typ, *typ2.elem_typ);
        default: UNREACHABLE;
    }
    UNREACHABLE;
}
