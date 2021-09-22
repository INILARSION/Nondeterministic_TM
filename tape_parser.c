#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tape_parser.h"
#include "tape_helper.h"
#include "program_helper.h"

/*
 * Get the amount of fields on the tape.
 */
int get_tape_length(const char *line, size_t line_length) {
    int tape_length = 1;
    for (int i = 0; i < line_length; ++i) {
        if (line[i] == '|')
            ++tape_length;
    }
    return tape_length;
}

/*
 * Get substring from the line, delimiter is '|'. Destination char* can be uninitialized.
 */
int get_state_substr_from_line(char *line, char **dest) {
    int tmp_str_size = 0;
    while (line[tmp_str_size] != '|' && line[tmp_str_size] != '\n' && line[tmp_str_size] != '\0')
        ++tmp_str_size;

    *dest = calloc(tmp_str_size + 1, sizeof(char));
    strncpy(*dest, line, tmp_str_size);
    return tmp_str_size;
}

/*
 * Get index of matching state name of first substring in the line.
 */
int search_matching_alphabet_element(struct program *program, char *line, int *alphabet_element) {
    char *tmp_str;
    int tmp_str_size = get_state_substr_from_line(line, &tmp_str);
    int found_match = -1;

    for (int j = 0; j < program->alphabet_size; ++j) {
        if(strcmp(tmp_str, program->alphabet[j]) == 0) {
            *alphabet_element = j;
            found_match = 0;
            break;
        }
    }
    if (found_match == -1) {
        printf("Tape contains element which is not contained in the alphabet!");
        exit(-1);
    }
    return tmp_str_size;
}

/*
 * This function parses a file containing one tape.
 */
struct tape *parse_tape(struct program *program){
    char *line = NULL;
    ssize_t line_length = 0;
    size_t buffer_size = 0;
    line_length = getline(&line, &buffer_size, stdin);

    struct tape *tape = malloc(sizeof(struct tape));

    if (line_length == -1)
        exit(-1);

    // exclude the newline
    if (line[line_length - 1] == '\n')
        --line_length;

    if (line_length == 0)
        tape->length = 0;

    tape->length = get_tape_length(line, line_length);
    tape->tape_arr = malloc(tape->length * sizeof(int));

    int alphabet_element_length;
    for (int i = 0; i < tape->length; ++i) {
        alphabet_element_length = search_matching_alphabet_element(program, line, &tape->tape_arr[i]);
        line += alphabet_element_length + 1;
    }

    return tape;
}