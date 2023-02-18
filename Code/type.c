#include "symtab.h"

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