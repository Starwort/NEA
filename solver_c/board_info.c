#include "board_info.h"

/* Determine if a stack is legal
 */
bool legal_stack(Column* col, uint8 position) {
    uint8 value = col->cards[position];
    for (uint8 y = position; y < col->count; y++) {
        if (col->cards[y] != value--) {
            return false;
        }
    }
    return true;
}

/* Determine where the stack begins
 */
uint8 stack_begin(Column* col) {
    // if there are no cards in the column then there is no stack
    if (col->count == 0) {
        return 0;
    }
    // if there is a cheated card then that is the entire stack
    if (col->cheated) {
        return col->count - 1;
    }
    uint8 y = col->count - 1;
    uint8 card = col->cards[y];
    while (--y != 255 && col->cards[y] == ++card) {
    }
    return y + 1;
}

/* Check if a given move is legal on a given board
 * Sets move->is_cheat
 */
bool can_move(Board* board, Move* move) {
    int from_col = move->from_x;
    int from_y = move->from_y;
    int to_col = move->to_x;
    Column* fcol = board->cols[from_col];
    Column* tcol = board->cols[to_col];

    // cards can never be moved onto a column which contains a Cheated card
    if (tcol->cheated) {
        return false;
    }

    // never move from the beginning of a stack to an empty stack
    if (fcol->stack_begin == 0 && tcol->count == 0) {
        return false;
    }

    // never move an invalid stack - this could actually be omitted as it's
    // impossible for such a Move* to be generated
    // if (from_y < fcol->count - 1 && !legal_stack(fcol, from_y)) {
    //     return false;
    // }

    move->is_cheat = false;  // assume not a cheat until we determine it is
    // if there are no cards in this column then the move is valid
    if (tcol->count == 0) {
        return true;
    }

    // otherwise check the top card and compare it to the card being moved
    if (tcol->cards[tcol->count - 1] == fcol->cards[from_y] + 1) {
        return true;
    }

    // if we reach here then the attempted move Cheats
    move->is_cheat = true;
    if (fcol->cheated) {
        // this is only a valid move if fcol->cards[fcol->count - 1] can
        // also be found in a legitimate position on a column that is neither
        // fcol nor tcol
        for (int i = 0; i < 6; i++) {
            if (i == from_col || i == to_col) {
                continue;
            }
            Column* col = board->cols[i];
            // the stack must be more than one card (the card must be
            // in a valid position)
            if (col->stack_begin < col->count - 1
                // and must end with the card we're trying to move
                && col->cards[col->count - 1] == fcol->cards[fcol->count - 1]) {
                return true;  // legal move (that is technically 2 moves)
            }
        }
        // not legal; no column permits this move
        return false;
    }
    // Cheating with a card that was previously not Cheated is always valid
    return true;
}

/* Determine if move is the special two-move operation Cheat -> Cheat
 * If it is, return the column of the intermediate column.
 * Else, return -1
 *
 * Assume that move is a legal Cheat move
 */
int column_if_two_moves(Board* board, Move* move) {
    int from_col = move->from_x;
    int from_y = move->from_y;
    int to_col = move->to_x;
    Column* fcol = board->cols[from_col];
    Column* tcol = board->cols[to_col];
    if (fcol->cheated) {
        // find the column using the algorithm from can_move
        for (int i = 0; i < 6; i++) {
            if (i == from_col || i == to_col) {
                continue;
            }
            Column* col = board->cols[i];
            if (col->stack_begin < col->count - 1
                && col->cards[col->count - 1] == fcol->cards[fcol->count - 1]) {
                return i;  // found the two-move column
            }
        }
    }
    return -1;  // this move cannot be Cheat -> Cheat
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
        if (board->cols[col]->count == 0  // column is empty
            || (board->cols[col]->count == 9  // or contains the full stack
                && board->cols[col]->stack_begin == 0)) {
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

/* Unapply move to board; opposite of apply_move
 */
void unapply_move(Board* board, Move* move) {
    Column* from_col = board->cols[move->from_x];
    Column* to_col = board->cols[move->to_x];
    uint8 n_elements = to_col->count - move->to_y;
    memcpy(from_col->cards + move->from_y, to_col->cards + move->to_y, n_elements);
    from_col->stack_begin = from_col->count;
    from_col->count = move->from_y + n_elements;
    to_col->count = move->to_y;
}
