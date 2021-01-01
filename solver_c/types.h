#pragma once
#include "common.h"

typedef uint8 Card;

typedef struct _Column {
    Card cards[14];
    uint8 count : 4;
    uint8 stack_begin : 4;
    bool cheated : 1;
} Column;

typedef struct _Board {
    Column* cols[6];  // Board.cols[x][y]
    int depth;
} Board;

typedef struct _Move {
    uint8 from_x : 4;
    uint8 from_y : 4;
    uint8 to_x : 3;
    uint8 to_y : 4;
    bool is_cheat : 1;
} Move;