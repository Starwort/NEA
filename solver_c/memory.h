#include "types.h"

Board* allocate_board();
void free_board(Board* board);
BoardHashTable_LLNode* create_node(const string board_state, int moves);
void deallocate_list(BoardHashTable_LLNode* node);
Move* copy_move(Move* move);
