#pragma once
#include "program_helper.h"
#include "tape_helper.h"

/*
 * Struct to save a Turing machine configuration
 */
struct node{
    struct deltas *delta;
    struct tape *tape;
    int head_position;
    // save father node to find all nodes from the accepting node to the root node
    struct node *father;
    struct node *child;
};

struct growable_queue {
    long head;
    long tail;
    long size;
    void **data;
};

void init_queue(struct growable_queue *queue, long initial_size);

void *pop_queue(struct growable_queue *queue);

int push_queue(struct growable_queue *queue, void *element);

int is_queue_empty(struct growable_queue *queue);