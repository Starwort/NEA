#pragma once
#include "common.h"

typedef uint8 Card;

typedef struct _Column {
    Card cards[14];
    uint8 count;
    uint8 stack_begin;
    bool cheated;
} Column;

typedef struct _Board {
    Column* cols[6];  // Board.cols[x][y]
    int depth;
} Board;

typedef struct _Move {
    uint8 from_x;
    uint8 from_y;
    uint8 to_x;
    uint8 to_y;
    bool is_cheat;
    bool was_cheat;
} Move;

typedef struct _BoardHashTable_LLNode {
    string board_state;
    int unsolvable_in;
    struct _BoardHashTable_LLNode* next;
} BoardHashTable_LLNode;
