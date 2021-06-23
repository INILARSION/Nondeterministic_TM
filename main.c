#include <stdio.h>
#include "tape_parser.h"
#include "tape_helper.h"
#include "program_parser.h"
#include "program_helper.h"
#include "simulator.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: %s [Tape file] [Program file]", argv[0]);
    }
    char *tape_file_path = argv[1];
    char *program_file_path = argv[2];

    struct program *program = parse_program(program_file_path);

    struct tape *tape = parse_tape(tape_file_path, program);

    simulate(tape, program);

    return 0;
}
