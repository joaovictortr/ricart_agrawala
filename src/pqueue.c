#include <stdio.h>
#include <malloc.h>
#include "pqueue.h"

int key_less(queue_item a, queue_item b);
void fixup_heap(pqueue q);
void fixdown_heap(pqueue q, int k);

int key_less(queue_item a, queue_item b)
{
    return a->pri < b->pri;
}

pqueue create_pqueue(int capacity)
{
    pqueue newq = malloc(sizeof(struct pqueue));
    if (newq == NULL)
        return NULL;

    newq->size = 0;
    newq->capacity = capacity;
    newq->content = malloc(sizeof(struct queue_item) * (capacity+1));

    if (newq->content == NULL) {
        free(newq);
        return NULL;
    }

    return newq;
}

int destroy_pqueue(pqueue q)
{
    if (q == NULL) return 0;

    if (q->content != NULL) {
        free(q->content);
    } else {
        fprintf(stderr, "[%s:%d] Priority queue content was not allocated!\n", __FUNCTION__, __LINE__);
    }
    free(q);
    return 1;
}

int empty_pqueue(pqueue q)
{
    return q->size == 0;
}

void fixup_heap(pqueue q)
{
    int k = q->size;
    int parent = k / 2;
    while(k > 1 && key_less(q->content[parent], q->content[k])) {
        queue_item t = q->content[parent];
        q->content[parent] = q->content[k];
        q->content[k] = t;
        k = parent;
        parent = k/2;
    }
}

void fixdown_heap(pqueue q, int k)
{
    int j, n = q->size-1;

    while(2*k <= n) {
        j = 2*k;
        if (j < n && key_less(q->content[j], q->content[j+1]))
            j++;
        if (!key_less(q->content[k], q->content[j]))
            break;
        queue_item t = q->content[k];
        q->content[k] = q->content[j];
        q->content[j] = t;
        k = j;
    }
}

int insert_pqueue(pqueue f, int pid, int timestamp, int pri, int type)
{
    if (f->size == f->capacity) {
        fprintf(stderr, "Priority queue is full! Item was not inserted.\n");
        return 0;
    }
    queue_item nitem = malloc(sizeof(struct queue_item));

    nitem->pid = pid;
    nitem->timestamp = timestamp;
    nitem->pri = pri;
    nitem->type = type;

    f->content[++f->size] = nitem;
    fixup_heap(f);
    return 1;
}


queue_item remove_max_pqueue(pqueue f)
{
    if (f == NULL) return NULL;

    if (empty_pqueue(f)) return NULL;

    queue_item t = f->content[1];
    f->content[1] = f->content[f->size];
    f->content[f->size] = t;

    fixdown_heap(f, 1);
    return f->content[f->size--];
}
