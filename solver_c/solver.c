#include "solver.h"

Move** moves;
BoardHashTable_LLNode* cache[0x1000000] = {NULL};
int cache_boundary = 16;
int continue_millis = 500;
Time timer = {0, 0};

/* Clear the cache. Used between no-Cheat and Cheat passes,
 * as well as at the end to prevent memory leaks
 */
void clear_cache() {
    eprintln("Clearing cache...");
    for (uint32 cache_idx = 0; cache_idx < 0x1000000;) {
        if (cache[cache_idx] != NULL) {
            deallocate_list(cache[cache_idx]);
            cache[cache_idx] = NULL;
        }
        cache_idx++;
    }
    eprintln("Done");
}

/* Search recursively for the next move; backtracking.
 * Return -1 if no valid moves found, non-negative to
 * indicate number of moves.
 *
 * Cheating should now be implemented
 */
int step(Board* board, int depth, int max_moves, bool allow_cheat) {
    if (depth >= max_moves) {
        return -1;
    }
    int remaining_moves = max_moves - depth;
    uint32 board_hash = hash(board) & 0xFFFFFF;
    BoardHashTable_LLNode* node = cache[board_hash];
    BoardHashTable_LLNode* last_node = NULL;
    while (node != NULL && !equal(board, (const string)node->board_state)) {
        node = (BoardHashTable_LLNode*)(last_node = node)->next;
    }
    if (node && node->unsolvable_in >= remaining_moves) {
        return -1;
    }
    bool free_node = false;
    if (node) {
        node->unsolvable_in = remaining_moves;
    } else {
        free_node = true;
        string compressed = compress(board);
        if (last_node) {
            last_node->next = create_node(compressed, remaining_moves);
            node = (BoardHashTable_LLNode*)last_node->next;
        } else {
            node = cache[board_hash] = create_node(compressed, remaining_moves);
        }
    }
    Move* move = malloc(sizeof(Move));
    Move* cheat_step_one;
    Move* cheat_step_two;
    int two_move_cheat_col;
    bool found_solution = false;
    // we want to start with non-Cheats only as these are more likely to work
    for (move->from_x = 0; move->from_x < 6; move->from_x++) {
        for (move->to_x = 0; move->to_x < 6; move->to_x++) {
            if (move->from_x == move->to_x) {
                continue;
            }
            if (board->cols[move->from_x]->count == 0) {
                continue;
            }
            // If we've run out of time, return; otherwise, keep looking for a shorter
            // solution
            if (expired(timer)) {
                goto finalise;
            }
            move->from_y = board->cols[move->from_x]->stack_begin;
            move->to_y = board->cols[move->to_x]->count;
            move->was_cheat = board->cols[move->from_x]->cheated;
            if (!can_move(board, move)) {
                continue;
            }
            if (move->is_cheat) {
                continue;
            }
            apply_move(board, move);
            if (solved(board)) {
                // Solving the board this move is the best I can do; start a timer and
                // return
                moves[depth] = move;
                // If the timer is not already running, start it
                start(timer, continue_millis);
                return depth + 1;
            }
            int found = step(board, depth + 1, max_moves, allow_cheat);
            if (found != -1) {
                // Found a solution in fewer moves than before
                moves[depth] = move;
                max_moves = found;
                // If the timer is not already running, start it
                found_solution = true;
                start(timer, continue_millis);
            }
            unapply_move(board, move);
        }
    }
    if (allow_cheat) {
        // we couldn't solve it without Cheating here; try Cheating now
        for (move->from_x = 0; move->from_x < 6; move->from_x++) {
            for (move->to_x = 0; move->to_x < 6; move->to_x++) {
                if (move->from_x == move->to_x) {
                    continue;
                }
                if (board->cols[move->from_x]->count == 0) {
                    continue;
                }
                // If we've run out of time, return; otherwise, keep looking for a
                // shorter solution
                if (expired(timer)) {
                    goto finalise;
                }
                move->from_y = board->cols[move->from_x]->stack_begin;
                move->to_y = board->cols[move->to_x]->count;
                move->was_cheat = board->cols[move->from_x]->cheated;
                if (!can_move(board, move)) {
                    continue;
                }
                if (!move->is_cheat) {
                    continue;
                }
                two_move_cheat_col = column_if_two_moves(board, move);
                if (two_move_cheat_col != -1) {
                    cheat_step_one = malloc(sizeof(Move));
                    cheat_step_two = malloc(sizeof(Move));
                    cheat_step_one->from_x = two_move_cheat_col;
                    cheat_step_one->from_y = board->cols[two_move_cheat_col]->count - 1;
                    cheat_step_one->to_x = move->to_x;
                    cheat_step_one->to_y = move->to_y;
                    cheat_step_one->is_cheat = true;
                    cheat_step_two->from_x = move->from_x;
                    cheat_step_two->from_y = move->from_y;
                    cheat_step_two->to_x = two_move_cheat_col;
                    cheat_step_two->to_y = board->cols[two_move_cheat_col]->count - 1;
                    cheat_step_two->is_cheat = false;
                    moves[depth++] = cheat_step_one;
                    moves[depth] = cheat_step_two;
                }
                apply_move(board, move);
                if (solved(board)) {
                    // Solving the board this move is the best I can do; start a timer
                    // and return
                    moves[depth] = move;
                    // If the timer is not already running, start it
                    start(timer, continue_millis);
                    return depth + 1;
                }
                int found = step(board, depth + 1, max_moves, allow_cheat);
                if (found != -1) {
                    // Found a solution in fewer moves than before
                    moves[depth] = move;
                    max_moves = found;
                    // If the timer is not already running, start it
                    found_solution = true;
                    start(timer, continue_millis);
                }
                unapply_move(board, move);
                if (two_move_cheat_col != -1) {
                    depth--;
                    free(cheat_step_one);
                    free(cheat_step_two);
                }
            }
        }
    }
finalise:
    // cut the node out if I had to allocate it
    if (free_node) {
        if (last_node) {
            last_node->next = node->next;
        } else {
            cache[board_hash] = NULL;
        }
        free((void*)node->board_state);
        free(node);
    }
    // if I found a solution, return the number of moves it took
    if (found_solution) {
        return max_moves;
    }
    // I didn't, so free the move to avoid a memory leak
    free(move);
    return -1;
}

int main(int argc, string argv[]) {
    extern string optarg;
    extern int optind, optopt;
    bool solver_allow_cheat = false;
    int max_depth = 1024;

    int opt;
    while ((opt = getopt(argc, argv, ":cn:t:m:")) != -1) {
        switch (opt) {
            case 'c':
                solver_allow_cheat = true;
                break;
            case 'n':
                cache_boundary = atoi(optarg);
                break;
            case 't':
                continue_millis = atoi(optarg);
                break;
            case 'm':
                max_depth = atoi(optarg);
                break;
            default:
                eprintfln(
                    "Usage: %s [-c] [-n <moves>=16] [-t <milliseconds>=500] [-m <max "
                    "moves>=1024] <STATE>",
                    argv[0]);
                eprintln("  -c                Allow Cheating if no solution is found "
                         "without Cheating");
                eprintln("  -n <cache moves>  How many moves may remain when deciding "
                         "whether to cache or not");
                eprintln("  -t <milliseconds> How long to continue searching for a "
                         "shorter solution after one is found");
                eprintln("  -m <max moves>    How many moves to allow in total. Should "
                         "be a power of 2 as other numbers will be truncated to the "
                         "previous power of 2");
                exit(EXIT_FAILURE);
        }
    }
    if (!argv[optind]) {
        println("Need starting board state");
        exit(EXIT_FAILURE);
    }
    Board* board = parse_input(argv[optind]);

    int start_max_depth = min(64, max_depth);
    moves = malloc(sizeof(Move*) * max_depth);

    for (int step_max_depth = start_max_depth; step_max_depth <= max_depth;
         step_max_depth <<= 1) {
        int n_moves = step(board, 0, step_max_depth, false);
        if (n_moves == -1) {
            eprintfln("No solution found in %d moves", step_max_depth);
        } else {
            for (int i = 0; i < n_moves; i++) {
                printfln(
                    "%d %d %c> %d %d",
                    moves[i]->from_x,
                    moves[i]->from_y,
                    moves[i]->is_cheat ? '!' : '-',
                    moves[i]->to_x,
                    moves[i]->to_y);
                free(moves[i]);
            }
            free_board(board);
            clear_cache();
            return 0;
        }
    }
    if (solver_allow_cheat) {
        clear_cache();
        for (int step_max_depth = start_max_depth; step_max_depth <= max_depth;
             max_depth <<= 1) {
            int n_moves = step(board, 0, step_max_depth, true);
            if (n_moves == -1) {
                eprintfln("No solution found in %d moves (Cheating)", step_max_depth);
            } else {
                for (int i = 0; i < n_moves; i++) {
                    printfln(
                        "%d %d %c> %d %d",
                        moves[i]->from_x,
                        moves[i]->from_y,
                        moves[i]->is_cheat ? '!' : '-',
                        moves[i]->to_x,
                        moves[i]->to_y);
                    free(moves[i]);
                }
                free_board(board);
                clear_cache();
                return 0;
            }
        }
    }
    free_board(board);
    clear_cache();
    return 0;
}
