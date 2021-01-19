#pragma once
#include "types.h"

bool legal_stack(Column* col, uint8 position);
uint8 stack_begin(Column* col);
bool can_move(Board* board, Move* move);
int column_if_two_moves(Board* board, Move* move);
bool solved(Board* board);
void apply_move(Board* board, Move* move);
void unapply_move(Board* board, Move* move);
