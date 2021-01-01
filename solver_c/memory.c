#include "memory.h"

/* Allocate a board and its columns
 * Needs freeing after use
 */
Board* allocate_board() {
    Board* board = malloc(sizeof(Board));
    board->depth = 0;
    for (int i = 0; i < 6; i++) {
        board->cols[i] = malloc(sizeof(Column));
    }
    return board;
}
/* Free a board and its columns
 */
void free_board(Board* board) {
    for (int i = 0; i < 6; i++) {
        free(board->cols[i]);
    }
    free(board);
}