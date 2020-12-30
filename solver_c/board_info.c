#include "board_info.h"

bool legal_stack(Column* col, uint8 position) {
    uint8 value = col->cards[position];
    for (uint8 y = position; y < col->count; y++) {
        if (col->cards[y] != value--) {
            return false;
        }
    }
    return true;
}

uint8 stack_begin(Column* col) {
    for (uint8 y = 0; y < col->count; y++) {
        if (legal_stack(col, y)) {
            return y;
        }
    }
}

bool can_move(Board* board, uint8 from_col, uint8 from_y, uint8 to_col) {
    Column* fcol = board->cols[from_col];
    Column* tcol = board->cols[to_col];
    if (from_y < fcol->count - 1) {
        if (legal_stack(fcol, from_y)) {
            return (tcol->cards[tcol->count] == fcol->cards[from_y] + 1);
        }
        return false;
    }
}
