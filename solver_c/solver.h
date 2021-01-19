#pragma once
#include "board_info.h"
// #include "debug.h"
#include "parse.h"
#include "types.h"

#include <argp.h>

const char* argp_program_version = "molek-syntez-solitaire-solver";
const char* argp_program_bug_address = "";
static char doc[] = "Solver for MOLEK-SYNTEZ solitaire minigame";
static char args_doc[] = "<STATE>";
static struct argp_option options[] = {
    {"cheat",
     'c',
     0,
     0,
     "Allow Cheating (only takes effect if "
     "no solution is found without Cheating)"},
    {0}};

struct arguments {
    bool cheat;
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
    struct arguments* arguments = state->input;
    switch (key) {
        case 'c':
            arguments->cheat = true;
            break;
        case ARGP_KEY_ARG:
            return 0;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

int step(Board* board, int depth, int max_moves, bool allow_cheat);
