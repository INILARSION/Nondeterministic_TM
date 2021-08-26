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
    if (((queue->head + queue->size) - queue->tail) % queue->size == queue->size - 1) {
        void **new_data = malloc(2 * queue->size * sizeof(struct deltas*));
        for (int i = 0; i < queue->size - 1; ++i) {
            new_data[i] = queue->data[(queue->tail + i) % queue->size];
        }
        queue->tail = 0;
        queue->head = queue->size - 1;
        queue->size *= 2;
        free(queue->data);
        queue->data = new_data;
    }
    queue->data[queue->head] = element;
    queue->head = ++queue->head % queue->size;
    return 0;
}

int is_queue_empty(struct growable_queue *queue) {
    return queue->tail == queue->head;
}