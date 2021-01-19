#include "types.h"

Board* allocate_board();
void free_board(Board* board);
BoardHashTable_LLNode* create_node(string board_state, int depth);
