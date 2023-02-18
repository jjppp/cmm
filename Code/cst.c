#include "cst.h"
#include "common.h"
#include "symtab.h"
#include <stdarg.h>

static void ins_chld(cst_t *node, cst_t *chld) {
    if (node->chld == NULL) {
        node->chld = chld;
        return;
    }
    cst_iter(node, it) {
        if (it->next == NULL) {
            it->next = chld;
            break;
        }
    }
}

cst_t *cst_alloc(const char *typ, const char *name, u32 fst_l, u32 nchld, ...) {
    va_list ap;
    va_start(ap, nchld);

    cst_t *node = zalloc(sizeof(cst_t));
    symcpy(node->typ, typ);
    symcpy(node->str, name);

    node->is_tok = (nchld == 0);
    node->fst_l  = fst_l;

    while (nchld--) {
        ins_chld(node, va_arg(ap, cst_t *));
    }

    va_end(ap);
    return node;
}

void cst_free(cst_t *node) {
    if (!node) {
        return;
    }
    cst_t *p = NULL;
    cst_iter(node, it) {
        cst_free(p);
        p = it;
    }
    cst_free(p);
    free(node);
}

void cst_print(cst_t *node, i32 dep) {
    if (node == NULL) {
        return;
    }
    for (i32 d = dep; d--;) {
        printf("  ");
    }
    const char *s = node->typ;
    if (!symcmp(s, "ID")) {
        printf("ID: %s\n", node->str);
    } else if (!symcmp(s, "INT")) {
        printf("INT: %s\n", node->str);
    } else if (!symcmp(s, "FLOAT")) {
        printf("FLOAT: %s\n", node->str);
    } else if (!symcmp(s, "TYPE")) {
        printf("TYPE: %s\n", node->str);
    } else if (node->is_tok) {
        printf("%s\n", node->typ);
    } else {
        printf("%s (%d)\n", node->typ, node->fst_l);
    }
    cst_iter(node, it) {
        cst_print(it, dep + 1);
    }
}