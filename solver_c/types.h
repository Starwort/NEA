#pragma once
#include "common.h"

typedef uint8 Card;

typedef struct _Column {
    // largest possible stack:
    // XXXXXTKDV09876C (15)
    Card cards[15];
    uint8 count;
    uint8 stack_begin;
    bool cheated;
} Column;

typedef struct _Board {
    Column* cols[6];  // Board.cols[x][y]
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
    const char* board_state;
    int unsolvable_in;
    const struct _BoardHashTable_LLNode* next;
} BoardHashTable_LLNode;
