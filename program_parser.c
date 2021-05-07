#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "program_parser.h"
#include "program_helper.h"

/*
 * Get the line count of a file.
 */
int get_file_line_count(FILE *file_ptr, char *line, size_t buffer_size) {
    int line_count = 0;
    //count lines and reset to beginning of file
    while (getline(&line, &buffer_size, file_ptr) != -1) {
        ++line_count;
    }
    fseek(file_ptr, 0, 0);
    return line_count;
}

/*
 * Parse state names and store it in the struct.
 * Sets start, halt and reject states.
 */
void parse_states(struct program *program, char *line, size_t line_length) {
    // check that at least one alphabet is present and that a program follows
    if (line_length < 4 || line[0] != 'S' || line[line_length-1] != '\n'){
        exit(-1);
    }
    // check some formatting
    if (line_length < 4 || line[0] != 'S') {
        printf("Wrong format for states!\n");
        exit(-1);
    }

    // exclude G: and \n
    line_length -= 4;
    // skip "S: "
    line += 3;

    // count number of states
    program->state_count = 1;
    for (int i = 0; i < line_length; ++i) {
        if (line[i] == ',')
            ++program->state_count;
    }

    program->state_names = malloc(program->state_count * sizeof(char*));

    int state_len;
    int current_state = 0;
    while (line[0] != '\0'){
        state_len = 0;
        while (line[state_len] != ',' && line[state_len] != '\n' && line[state_len] != '\0')
            ++state_len;

        char *state_name = calloc(state_len + 1, sizeof(char));
        strncpy(state_name, line, state_len);

        program->state_names[current_state] = state_name;
        ++current_state;

        line = line + state_len + 1;
    }

    program->start_state = 0;
    program->accept_state = 1;
    program->reject_state = 2;
}

/*
 * Parse alphabet part of the program file. Add its parts to the program struct.
 */
void parse_alphabet(struct program *program, char *line, size_t line_length) {
    // check that at least one alphabet is present and that a program follows
    if (line_length < 4 || line[0] != 'G' || line[line_length-1] != '\n'){
        exit(-1);
    }
    // exclude G: and \n
    line_length -= 4;
    // skip "G: "
    line += 3;

    // get the number of alphabet symbols
    program->alphabet_size = 1;
    for (int i = 0; i < line_length; ++i) {
        if (line[i] == ',')
            ++program->alphabet_size;
    }

    program->alphabet = malloc(program->alphabet_size * sizeof(char*));

    // get the length of the individual alphabet symbols and put them then in a list
    int alphabet_symbol_len;
    int current_element = 0;
    while (line[0] != '\0'){
        // get the length of the current alphabet symbol
        alphabet_symbol_len = 0;
        while (line[alphabet_symbol_len] != ',' && line[alphabet_symbol_len] != '\n' && line[alphabet_symbol_len] != '\0')
            ++alphabet_symbol_len;

        // copy the symbol from the whole line in a dedicated var
        char *alphabet_symbol = calloc(alphabet_symbol_len + 1, sizeof(char));
        strncpy(alphabet_symbol, line, alphabet_symbol_len);

        // add symbol to the list
        program->alphabet[current_element] = alphabet_symbol;
        ++current_element;

        line = line + alphabet_symbol_len + 1;
    }
}

/*
 * Get substring from the line, delimiter ist ','. Destination char* can be uninitialized.
 */
int get_substr_from_line(char *line, char **dest) {
    int tmp_str_size = 0;
    while (line[tmp_str_size] != ',' && line[tmp_str_size] != '\n' && line[tmp_str_size] != '\0')
        ++tmp_str_size;
    *dest = calloc(tmp_str_size + 1, sizeof(char));
    strncpy(*dest, line, tmp_str_size);
    return tmp_str_size;
}

/*
 * Get index of matching state name of first substring in the line.
 */
int search_matching_element(char *line, char **elements, int element_count, int *dest) {
    char *tmp_str;
    // get first substring from the whole line
    int tmp_str_size = get_substr_from_line(line, &tmp_str);
    int found_match = -1;
    // compare substring with the list of elements and get the index if found
    for (int j = 0; j < element_count; ++j) {
        if(strcmp(tmp_str, elements[j]) == 0) {
            *dest = j;
            found_match = 0;
            break;
        }
    }
    if (found_match == -1) {
        printf("Program delta contains state which is not contained in the defined states!");
        exit(-1);
    }
    return tmp_str_size;
}

/*
 * Parse delta part of the program file. Add an array of delta structs to the program struct.
 */
void parse_deltas(struct program *program, FILE *file_ptr, int line_count) {
    size_t line_length;
    size_t buffer_size = 0;
    char *line;
    int tmp_str_size;
    program->deltas_count = line_count;
    program->deltas = malloc(line_count * sizeof(struct deltas));
    for (int i = line_count; i > 0; --i) {
        line_length = getline(&line, &buffer_size, file_ptr);

        // check formatting and size
        if (line_length < 12 || line[0] != 'D'){
            printf("Delta has wrong formatting!");
            exit(-1);
        }

        // skip "D: "
        line += 3;

        struct deltas delta;

        // get state name
        tmp_str_size = search_matching_element(line, program->state_names, program->state_count, &delta.state);
        line += tmp_str_size + 1;

        // get subsequent state name
        tmp_str_size = search_matching_element(line, program->state_names, program->state_count, &delta.subsequent_state);
        line += tmp_str_size + 1;

        // get read symbols
        tmp_str_size = search_matching_element(line, program->alphabet, program->alphabet_size, &delta.read_symbol);
        line += tmp_str_size + 1;

        // get write symbols
        tmp_str_size = search_matching_element(line, program->alphabet, program->alphabet_size, &delta.write_symbol);
        line += tmp_str_size + 1;

        delta.movement = *line;

        program->deltas[i - 1] = delta;
    }
}

/*
 * Create an array where each index corresponds to a state name from the state_name array.
 * Each entry contains an array of pointers, which point to the specific delta, which has the corresponding state name.
 * Sorting is useful for the creation of the tree/breadth first search data structure.
 */
void sort_deltas_by_state(struct program *program) {
    program->state_delta_mapping = malloc(program->state_count * sizeof(struct sorted_deltas));

    int deltas_with_state_count;
    for (int i = 0; i < program->state_count; ++i) {
        // count number of deltas with states with i state name index
        deltas_with_state_count = 0;
        for (int j = 0; j < program->deltas_count; ++j) {
            if (program->deltas[j].state == i)
                ++deltas_with_state_count;
        }

        program->state_delta_mapping[i].same_state_count = deltas_with_state_count;

        // There will be states where no delta with this state (ie the accept state)
        // just continue and set the list to NULL
        if (deltas_with_state_count == 0) {
            program->state_delta_mapping[i].deltas_same_state = NULL;
            continue;
        }

        // put all deltas with state i in an array
        program->state_delta_mapping[i].deltas_same_state = malloc(deltas_with_state_count * sizeof(struct deltas*));
        for (int j = 0; j < program->deltas_count; ++j) {
            if (program->deltas[j].state == i)
                program->state_delta_mapping[i].deltas_same_state[j] = &program->deltas[j];
        }
    }
}

/*
 * Parse the file containing the TM-Program and produce a struct containing its information.
 */
struct program parse_program(char *program_file_path) {
    FILE *file_ptr = fopen(program_file_path, "r");
    size_t line_length;
    char *line = NULL;
    size_t buffer_size = 0;
    struct program program;

    // check that file is found
    if(file_ptr == NULL) {
        printf("File %s not found!", program_file_path);
        exit(1);
    }

    //count lines and reset to beginning of file
    int line_count = get_file_line_count(file_ptr, line, buffer_size);

    // check if enough lines are contained, line for states, alphabet and at least one transition has to be present
    if(line_count < 3) {
        printf("File does not contain enough lines!");
        exit(1);
    }

    // parse states from next line
    line_length = getline(&line, &buffer_size, file_ptr);
    parse_states(&program, line, line_length);
    --line_count;

    // parse alphabet from next line
    line_length = getline(&line, &buffer_size, file_ptr);
    parse_alphabet(&program, line, line_length);
    --line_count;

    // get all deltas/transitions of the program
    parse_deltas(&program, file_ptr, line_count);

    if (line)
        free(line);
    fclose(file_ptr);

    sort_deltas_by_state(&program);

    return program;
}