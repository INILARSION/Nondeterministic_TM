#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "simulator.h"
#include "tape_helper.h"
#include "program_helper.h"
#include "simulator_helper.h"

/*
 * Helper function to print the content of the tape
 */
void print_tape(struct program *program, struct tape *tape, char *prefix, int head_position) {
    int head_offset = strlen(prefix) + 2;
    //print tape
    printf("%s: ", prefix);
    for (int i = 0; i < tape->length; ++i) {
        // calculate string offset of head position
        if (i < head_position) {
            head_offset += strlen(program->alphabet[tape->tape_arr[i]]) + 1;
        }
        printf("%s", program->alphabet[tape->tape_arr[i]]);
        // print delimiter between tape symbols
        if (i < tape->length - 1)
            printf("|");
    }
    printf("\n");

    //print head position
    for (int i = 0; i < head_offset; ++i) {
        printf(" ");
    }
    printf("^\n\n");
}

/*
 * This function prints all tape configurations from the root node to the accepted node
 */
void print_all_configurations(struct node *current_node, struct program *program) {
    // accepted node is last node so has no child
    current_node->child = NULL;
    // go to the root node and set the child node
    struct node *tmp_node;
    while (current_node->father != NULL) {
        tmp_node = current_node->father;
        tmp_node->child = current_node;
        current_node = tmp_node;
    }
    // go from the root node (now current_node) and go to the accepted node and print the tapes on the way
    while (current_node->child != NULL) {
        printf(
                "Delta: %s,%s,%s,%s,%c\n",
                program->state_names[current_node->delta->state],
                program->alphabet[current_node->delta->read_symbol],
                program->state_names[current_node->delta->subsequent_state],
                program->alphabet[current_node->delta->write_symbol],
                current_node->delta->movement
                );
        print_tape(program, current_node->tape, "Tape", current_node->head_position);
        current_node = current_node->child;
    }
}

/*
 * This function runs the simulation for a node.
 * The symbol on the tape under the head gets changed to the write symbol.
 * The head is moved and tape expanded if needed.
 * New Nodes are added if applicable following deltas are present.
 */
void apply_delta(struct program *program, struct node *current_node, struct growable_queue *queue) {
    // change symbol on tape for write symbol
    current_node->tape->tape_arr[current_node->head_position] = current_node->delta->write_symbol;
    // move head
    switch (current_node->delta->movement) {
        // move head right. Expand the tape if head is now out of bounds.
        case '>':
            ++current_node->head_position;
            // if head index out of bounds of tape make a bigger tape, copy old tape, add empty symbol to last field of tape
            if (current_node->head_position == current_node->tape->length){
                int *new_tape = malloc((current_node->tape->length + 1) * sizeof(int));
                // copy existing tape starting index 0. Last index will be the empty element.
                memcpy(new_tape, current_node->tape->tape_arr, current_node->tape->length * sizeof(int));
                new_tape[current_node->head_position] = 0;
                free(current_node->tape->tape_arr);
                ++current_node->tape->length;
                current_node->tape->tape_arr = new_tape;
            }
            break;
        // move head left. Expand the tape if head is now out of bounds.
        case '<':
            --current_node->head_position;
            // if head index out of bounds of tape make a bigger tape, copy old tape, add empty symbol to first field of tape
            if (current_node->head_position == -1){
                current_node->head_position = 0;
                int *new_tape = malloc((current_node->tape->length + 1) * sizeof(int));
                // copy existing tape starting index 1. Index 0 will be the empty element.
                memcpy(new_tape + 1, current_node->tape->tape_arr, current_node->tape->length * sizeof(int));
                new_tape[0] = 0;
                current_node->head_position = 0;
                free(current_node->tape->tape_arr);
                ++current_node->tape->length;
                current_node->tape->tape_arr = new_tape;
            }
            break;
    }

    // check if next state is accept state
    if (current_node->delta->subsequent_state == program->accept_state) {
        queue->head = 0;
        queue->tail = 0;
        if (program->is_verbose)
            print_all_configurations(current_node, program);
        print_tape(program, current_node->tape, "Final tape", current_node->head_position);
        printf("Turing machine halted in an accepting state\n");
        exit(0);
    } else if (current_node->delta->subsequent_state == program->reject_state) {
        return;
    }

    struct sorted_deltas *deltas_subsequent_state = &program->state_delta_mapping[current_node->delta->subsequent_state];

    // add all deltas of the subsequent state in the queue
    for (int i = 0; i < deltas_subsequent_state->same_state_count; ++i) {
        if (current_node->tape->tape_arr[current_node->head_position] == deltas_subsequent_state->deltas_same_state[i]->read_symbol) {
            struct node *new_node = malloc(sizeof(struct node));
            new_node->delta = deltas_subsequent_state->deltas_same_state[i];
            new_node->tape = malloc(sizeof(struct tape));
            new_node->tape->length = current_node->tape->length;
            new_node->tape->tape_arr = malloc(current_node->tape->length * sizeof(int));
            memcpy(new_node->tape->tape_arr, current_node->tape->tape_arr, current_node->tape->length * sizeof(int));
            new_node->head_position = current_node->head_position;
            new_node->father = current_node;
            push_queue(queue, new_node);
        }
    }
}

/*
 * Helper function to run the simulation of node.
 * Get first node from queue, delegate simulation and free memory after node is used.
 */
void run_partial_simulation(struct program *program, struct growable_queue *queue) {
    struct node *current_node = pop_queue(queue);

    apply_delta(program, current_node, queue);

    // If the program is not verbose free the memory
    // If it is verbose, the memory is needed for the backtracking
    if (!program->is_verbose) {
        free(current_node->tape->tape_arr);
        free(current_node->tape);
        free(current_node);
    }
}

/*
 * This function starts the simulation of the Non-deterministic Turing machine.
 */
void simulate(struct tape *tape, struct program *program) {
    // Create and initialize a FIFO queue, which contains the nodes (configuration of delta, tape and head position) of the computation tree
    // FIFO is needed because the "tree" which emerges from the nondeterminism has to be traversed as BFS
    struct growable_queue queue;
    init_queue(&queue, 10000);

    // Get all deltas from the first state
    struct sorted_deltas *deltas_start_state = &program->state_delta_mapping[program->start_state];

    // Create nodes and put all deltas with start state in queue
    for (int i = 0; i < deltas_start_state->same_state_count; ++i) {
        if (tape->tape_arr[0] == deltas_start_state->deltas_same_state[i]->read_symbol) {
            struct node *start_node = malloc(sizeof(struct node));
            start_node->delta = deltas_start_state->deltas_same_state[i];
            start_node->tape = malloc(sizeof(struct tape));
            start_node->tape->length = tape->length;
            start_node->tape->tape_arr = calloc(tape->length, sizeof(int));
            memcpy(start_node->tape->tape_arr, tape->tape_arr, tape->length * sizeof(int));
            start_node->head_position = 0;
            // root nodes have no father
            start_node->father = NULL;
            push_queue(&queue, start_node);
        }
    }

    // Run simulations on each node of the queue
    // Each simulation can add nodes to the queue if it has children nodes in the tree
    // Queue will be set to empty if an accepted state is found
    while (!is_queue_empty(&queue))
        run_partial_simulation(program, &queue);

    printf("Turing machine halted in a not accepting state!\n");
}