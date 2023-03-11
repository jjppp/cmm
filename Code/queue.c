#include "queue.h"
#include "cfg.h"

void queue_init(queue_t *q, u32 bound) {
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    q->inq  = zalloc(bound * sizeof(bool));
}

void queue_fini(queue_t *q) {
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    zfree(q->inq);
}

void queue_push(queue_t *q, block_t *blk) {
    if (q->inq[blk->id]) {
        return;
    }
    queue_elem_t *elem = zalloc(sizeof(queue_elem_t));

    elem->blk = blk;
    if (q->size == 0) {
        q->head = elem;
    } else {
        q->tail->next = elem;
    }
    q->tail = elem;
    q->size++;
    q->inq[blk->id] = true;
}

void queue_push_front(queue_t *q, block_t *blk) {
    if (q->inq[blk->id]) {
        return;
    }
    queue_elem_t *elem = zalloc(sizeof(queue_elem_t));

    elem->blk = blk;
    if (q->size == 0) {
        q->tail = elem;
    }
    elem->next = q->head;
    q->head    = elem;
    q->size++;
    q->inq[blk->id] = true;
}

block_t *queue_pop(queue_t *q) {
    block_t *blk = q->head->blk;
    void    *ptr = q->head;
    q->head      = q->head->next;
    q->size--;
    if (queue_empty(q)) {
        q->tail = NULL;
    }
    q->inq[blk->id] = false;
    zfree(ptr);
    return blk;
}

bool queue_empty(queue_t *q) {
    return q->size == 0;
}