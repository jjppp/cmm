#pragma once

#include "cfg.h"

typedef struct queue_elem_t queue_elem_t;
typedef struct queue_t      queue_t;

struct queue_elem_t {
    block_t      *blk;
    queue_elem_t *next;
};

struct queue_t {
    queue_elem_t *head, *tail;
    bool         *inq;
    u32           size;
};

void queue_init(queue_t *q, u32 bound);

void queue_fini(queue_t *q);

void queue_push(queue_t *q, block_t *blk);

block_t *queue_pop(queue_t *q);

bool queue_empty(queue_t *q);