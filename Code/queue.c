#include "queue.h"
#include "cfg.h"

void queue_init(queue_t *q) {
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

void queue_push(queue_t *q, block_t *blk) {
    queue_elem_t *elem = zalloc(sizeof(queue_elem_t));

    elem->blk = blk;
    if (q->size == 0) {
        q->head = elem;
    } else {
        q->tail->next = elem;
    }
    q->tail = elem;
    q->size++;
}

block_t *queue_pop(queue_t *q) {
    block_t *blk = q->head->blk;
    void    *ptr = q->head;
    q->head      = q->head->next;
    q->size--;
    if (queue_empty(q)) {
        q->tail = NULL;
    }
    zfree(ptr);
    return blk;
}

bool queue_empty(queue_t *q) {
    return q->size == 0;
}