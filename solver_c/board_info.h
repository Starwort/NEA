#pragma once
#include "types.h"

bool legal_stack(const Column* col, uint8 position);
uint8 stack_begin(const Column* col);
bool can_move(const Board* board, Move* move);
int column_if_two_moves(const Board* board, const Move* move);
bool solved(const Board* board);
void apply_move(Board* board, const Move* move);
void unapply_move(Board* board, const Move* move);
