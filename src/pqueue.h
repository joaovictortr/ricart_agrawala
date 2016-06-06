#ifndef __PQUEUE_H_
#define __PQUEUE_H_
struct queue_item {
    int pri;
    int pid;
    int timestamp;
    int type;
};
typedef struct queue_item *queue_item;

struct pqueue {
    int size;
    int capacity;
    queue_item *content;
};

typedef struct pqueue* pqueue;

pqueue create_pqueue(int capacity);
int destroy_pqueue(pqueue q);
int insert_pqueue(pqueue f, int pid, int timestamp, int pri, int type);
queue_item remove_max_pqueue(pqueue f);
#endif
