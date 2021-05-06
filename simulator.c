#include <malloc.h>
#include <string.h>
#include "simulator.h"
#include "tape_helper.h"
#include "program_helper.h"
#include "simulator_helper.h"

void print_tape(struct tape *tape, struct program *program) {
    printf("Final tape: ");
    for (int i = 0; i < tape->length; ++i) {
        printf("%s", program->alphabet[tape->tape_arr[i]]);
        if (i < tape->length - 1)
            printf(",");
    }
    printf("\n");
}

void apply_delta(struct program *program, struct node *current_node, struct growable_queue *queue) {
    // change symbol on tape for write symbol
    current_node->tape->tape_arr[current_node->head_position] = current_node->delta->write_symbol;
    // move head
    switch (current_node->delta->movement) {
        case '>':
            ++current_node->head_position;
            // if head index out of bounds of tape make a bigger tape, copy old tape, add empty symbol to last field of tape
            if (current_node->head_position == current_node->tape->length){
                int *new_tape = malloc((current_node->tape->length + 1) * sizeof(int));
                memcpy(new_tape, current_node->tape->tape_arr, current_node->tape->length * sizeof(int));
                new_tape[current_node->head_position] = 0;
                free(current_node->tape->tape_arr);
                current_node->tape->tape_arr = new_tape;
            }
            break;
        case '<':
            --current_node->head_position;
            // if head index out of bounds of tape make a bigger tape, copy old tape, add empty symbol to first field of tape
            if (current_node->head_position == -1){
                current_node->head_position = 0;
                int *new_tape = malloc((current_node->tape->length + 1) * sizeof(int));
                memcpy(new_tape + 1, current_node->tape->tape_arr, current_node->tape->length * sizeof(int));
                new_tape[0] = 0;
                current_node->head_position = 0;
                free(current_node->tape->tape_arr);
                current_node->tape->tape_arr = new_tape;
            }
            break;
    }

    // check if next state is accept state
    if (current_node->delta->subsequent_state == program->accept_state) {
        queue->head = 0;
        queue->tail = 0;
        printf("Accepted state!\n");
        print_tape(current_node->tape, program);
        return;
    } else if (current_node->delta->subsequent_state == program->reject_state) {
        return;
    }

    // add all deltas of the subsequent state in the queue
    for (int i = 0; i < program->deltas_same_state_count[current_node->delta->subsequent_state]; ++i) {
        if (current_node->tape->tape_arr[current_node->head_position] == program->state_delta_mapping[current_node->delta->subsequent_state][i]->read_symbol) {
            struct node *new_node = malloc(sizeof(struct node));
            new_node->delta = program->state_delta_mapping[current_node->delta->subsequent_state][i];
            new_node->tape = malloc(sizeof(struct tape));
            new_node->tape->length = current_node->tape->length;
            new_node->tape->tape_arr = malloc(current_node->tape->length * sizeof(int));
            memcpy(new_node->tape->tape_arr, current_node->tape->tape_arr, current_node->tape->length * sizeof(int));
            new_node->head_position = current_node->head_position;
            push_queue(queue, new_node);
        }
    }
}

void run_partial_simulation(struct program *program, struct growable_queue *queue) {
    struct node *current_node = pop_queue(queue);

    apply_delta(program, current_node, queue);

    free(current_node->tape->tape_arr);
    free(current_node->tape);
    free(current_node);
}

void simulate(struct tape *tape, struct program *program) {
    struct growable_queue queue;
    init_queue(&queue, 50);

    // put all deltas with start state in queue
    for (int i = 0; i < program->deltas_same_state_count[program->start_state]; ++i) {
        if (tape->tape_arr[0] == program->state_delta_mapping[program->start_state][i]->read_symbol) {
            struct node *start_node = malloc(sizeof(struct node));
            start_node->delta = program->state_delta_mapping[program->start_state][i];
            start_node->tape = malloc(sizeof(struct tape));
            start_node->tape->length = tape->length;
            start_node->tape->tape_arr = calloc(tape->length, sizeof(int));
            memcpy(start_node->tape->tape_arr, tape->tape_arr, tape->length * sizeof(int));
            start_node->head_position = 0;
            push_queue(&queue, start_node);
        }
    }

    while (!is_queue_empty(&queue))
        run_partial_simulation(program, &queue);
}