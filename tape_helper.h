#pragma once

/*
 * This structure encodes a standard tape as a string with its size
 */
struct tape {
    int length;
    int *tape_arr;
};