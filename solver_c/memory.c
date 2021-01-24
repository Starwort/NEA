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
BoardHashTable_LLNode* create_node(const string board_state, int moves) {
    BoardHashTable_LLNode* node = malloc(sizeof(BoardHashTable_LLNode));
    node->board_state = board_state;
    node->unsolvable_in = moves;
    node->next = NULL;
    return node;
}

/* Completely deallocate a BoardHashTable linked list
 *
 * Should get a Tail Recursion Optimisation
 */
void deallocate_list(BoardHashTable_LLNode* node) {
    if (node == NULL) {
        return;
    }
    BoardHashTable_LLNode* next = (BoardHashTable_LLNode*)node->next;
    free((void*)node->board_state);
    free(node);
    deallocate_list(next);
}
