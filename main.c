#include <stdio.h>
#include <string.h>
#include "tape_parser.h"
#include "tape_helper.h"
#include "program_parser.h"
#include "program_helper.h"
#include "simulator.h"

int main(int argc, char** argv) {

    char *tape_file_path;
    char *program_file_path;
    int is_verbose;

    if (argc == 3) {
        is_verbose = 0;
        tape_file_path = argv[1];
        program_file_path = argv[2];
    } else if (argc == 4) {
        if (strcmp(argv[1], "-v") != 0) {
            printf("Usage: %s [optional flag: -v] [Tape file] [Program file]", argv[0]);
            return -1;
        }
        is_verbose = 1;
        tape_file_path = argv[2];
        program_file_path = argv[3];
    } else {
        printf("Usage: %s [optional flag: -v] [Tape file] [Program file]", argv[0]);
        return -1;
    }

    struct program *program = parse_program(program_file_path);
    program->is_verbose = is_verbose;

    struct tape *tape = parse_tape(tape_file_path, program);

    simulate(tape, program);

    return 0;
}
