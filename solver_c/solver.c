#include "common.h"

typedef struct _Card {
    int8 value;
} Card;
typedef struct _Column {
    Card* cards[14];
    uint8 count : 3;
    uint8 stack_begin : 3;
    bool cheated : 1;
} Column;

typedef struct _Board {
    Column* cols[6];  // Board.cols[x][y]
    int depth;
} Board;

typedef struct _Move {
    uint8 from_x : 3;
    uint8 from_y : 4;
    uint8 to_x : 3;
    bool is_cheat : 1;
} Move;
bool legal_stack(Column* col, int position) {
    int value = col->cards[position]->value;
    for (int y = position; y < col->count; y++) {
        if (col->cards[y]->value != value--) {
            return false;
        }
    }
    return true;
}
bool can_move(Board* board, int from_col, int from_y, int to_col) {
    Column* fcol = board->cols[from_col];
    Column* tcol = board->cols[to_col];
    if (from_y < fcol->count - 1) {
        if (legal_stack(fcol, from_y)) {
            return (tcol->cards[tcol->count]->value == fcol->cards[from_y]->value + 1);
        }
        return false;
    }
}

void get_moves(Board* board, Move* out[]) {
    //
}

int main(int argc, string argv[]) {
    //
    return 0;
}