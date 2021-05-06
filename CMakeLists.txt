cmake_minimum_required(VERSION 3.19)
project(Nichtdeterministische_TM C)

set(CMAKE_C_STANDARD 11)

add_executable(Nichtdeterministische_TM main.c tape_parser.c tape_parser.h program_parser.c program_parser.h simulator.c simulator.h tape_helper.h program_helper.h simulator_helper.h simulator_helper.c simulator_helper.c simulator_helper.h)