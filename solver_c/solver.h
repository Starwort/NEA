#pragma once
#include "board_info.h"
#include "compress.h"
#include "hash.h"
#include "memory.h"
#include "parse.h"
#include "timer.h"
#include "types.h"

#include <unistd.h>

int step(Board* board, int depth, int max_moves, bool allow_cheat);
