#include "simulator_helper.h"
#include "program_helper.h"
#include <stdlib.h>

void init_queue(struct growable_queue *queue, long initial_size) {
    queue->head = 0;
    queue->tail = 0;
    queue->size = initial_size;
    queue->data = malloc(initial_size * sizeof(struct deltas*));
}

void *pop_queue(struct growable_queue *queue) {
    if (queue->tail == queue->head) {
        return NULL;
    }
    void *handle = queue->data[queue->tail];
    queue->data[queue->tail] = NULL;
    queue->tail = (queue->tail + 1) % queue->size;
    return handle;
}

int push_queue(struct growable_queue *queue, void *element) {
    if ((queue->head - queue->tail) == queue->size) {
        queue->head = queue->head % queue->size;
        queue->tail = queue->tail % queue->size;
        if (queue->head < queue->tail)
            queue->head += queue->size;
        queue->size *= 2;
        queue->data = realloc(queue->data, queue->size * sizeof(struct deltas*));
    }
    queue->data[queue->head] = element;
    queue->head = (queue->head + 1) % queue->size;
    return 0;
}

int is_queue_empty(struct growable_queue *queue) {
    return queue->tail == queue->head;
}