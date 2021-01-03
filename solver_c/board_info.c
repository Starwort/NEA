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
    if (col->count == 0) {
        return 0;
    }
    if (col->cheated) {
        return col->count - 1;
    }
    uint8 y = col->count - 1;
    uint8 card = col->cards[y];
    while (col->cards[--y] == ++card && y != 255) {
    }
    return y + 1;
}

bool can_move(Board* board, uint8 from_col, uint8 from_y, uint8 to_col) {
    Column* fcol = board->cols[from_col];
    Column* tcol = board->cols[to_col];
    if (tcol->cheated) {
        return false;
    }
    if (fcol->stack_begin == 0 && tcol->count == 0) {
        // never move from the beginning of a stack to an
        // empty stack
        return false;
    }
    if (from_y < fcol->count - 1 && !legal_stack(fcol, from_y)) {
        return false;
    }
    if (tcol->count == 0) {
        return true;
    }
    return (tcol->cards[tcol->count - 1] == fcol->cards[from_y] + 1);
}

/* Determine if the board is solved
 *
 * The board is solved if all columns are either empty or
 * contain a full stack starting from the top of the tableau
 * (col->cards == {14, 13, 12, 11, 10, 9, 8, 7, 6} and
 * therefore col->count == 9 and col->stack_begin == 0)
 */
bool solved(Board* board) {
    for (int col = 0; col < 6; col++) {
        if (board->cols[col]->count == 0
            || (board->cols[col]->count == 9 && board->cols[col]->stack_begin == 0)) {
            continue;
        }
        return false;
    }
    return true;
}

/* Apply move to board.
 *
 * Does not check validity of move.
 */
void apply_move(Board* board, Move* move) {
    Column* from_col = board->cols[move->from_x];
    Column* to_col = board->cols[move->to_x];
    uint8 n_elements = from_col->count - move->from_y;
    memcpy(to_col->cards + move->to_y, from_col->cards + move->from_y, n_elements);
    from_col->count = move->from_y;
    from_col->stack_begin = stack_begin(from_col);
    to_col->count += n_elements;
}

/* Unapply move to board; opposite of apply_move */
void unapply_move(Board* board, Move* move) {
    Column* from_col = board->cols[move->from_x];
    Column* to_col = board->cols[move->to_x];
    uint8 n_elements = to_col->count - move->to_y;
    memcpy(from_col->cards + move->from_y, to_col->cards + move->to_y, n_elements);
    from_col->stack_begin = from_col->count;
    from_col->count = move->from_y + n_elements;
    to_col->count = move->to_y;
}
