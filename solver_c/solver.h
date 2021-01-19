#pragma once
#include "board_info.h"
#include "compress.h"
#include "hash.h"
#include "memory.h"
#include "parse.h"
#include "types.h"

#include <unistd.h>

#define MAX_DEPTH 1024

int step(Board* board, int depth, int max_moves, bool allow_cheat);
