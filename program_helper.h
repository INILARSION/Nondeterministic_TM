#pragma once

/*
 * Struct contains one delta/transition of the program
 */
struct deltas {
    // index of state name in state_names in program struct
    int state;
    // index of subsequent state name in state_names in program struct
    int subsequent_state;
    // index of read alphabet element in alphabet in program struct
    int read_symbol;
    // index of write alphabet element in alphabet in program struct
    int write_symbol;
    // Move direction of head. Can be '<', '>' and '-'.
    char movement;
};

/*
 * Struct contains all information of the TM-Program
 */
struct program {
    // Array of all state names.
    char **state_names;
    // Number of different states.
    int state_count;
    int start_state;
    int accept_state;
    int reject_state;
    // Array of all elements of the alphabet. Elements are strings.
    char **alphabet;
    // Number of different alphabet elements
    int alphabet_size;
    // Array of all deltas/transitions of the Program
    struct deltas *deltas;
    //Number of all deltas/transitions
    int deltas_count;
    // list of list of delta pointers
    // each list of delta pointers contains all deltas of the same state name
    // these lists are stored in a list by state name. where the index corresponds to the state_names index.
    // Size is deltas_count
    struct deltas ***state_delta_mapping;
    // Array of number of deltas with same state name. Saves size for each state_delta_mapping array entry.
    // Size is deltas_count.
    // Index corresponds to state name of state_names.
    int *deltas_same_state_count;
};
