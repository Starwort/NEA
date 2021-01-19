#pragma once
#include "board_info.h"
// #include "debug.h"
#include "parse.h"
#include "types.h"

#include <unistd.h>

int step(Board* board, int depth, int max_moves, bool allow_cheat);
