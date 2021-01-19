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

/* Allocate a BoardHashTable_LLNode
 */
BoardHashTable_LLNode* create_node(string board_state, int depth) {
    BoardHashTable_LLNode* node = malloc(sizeof(BoardHashTable_LLNode));
    node->board_state = board_state;
    node->unsolvable_in = depth;
    node->next = NULL;
    return node;
}
