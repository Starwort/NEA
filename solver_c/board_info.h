#pragma once
#include "types.h"

bool legal_stack(Column* col, uint8 position);
uint8 stack_begin(Column* col);
bool can_move(Board* board, uint8 from_col, uint8 from_y, uint8 to_col);
