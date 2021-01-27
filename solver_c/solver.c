#include "solver.h"

Move** moves;
BoardHashTable_LLNode* cache[0x1000000] = {NULL};
int cache_boundary = 16;
int continue_millis = 500;
int max_depth = 1024;
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

/* Free moves[idx] if required, and return whether moves[idx] was freed */
bool free_if_required(int idx) {
    if (moves[idx] != NULL) {
        free(moves[idx]);
        moves[idx] = NULL;
        return true;
    }
    return false;
}

/* Free all moves from idx onwards
 */
void free_moves_from(int idx) {
    // Loop all elements from idx to the end
    for (int i = idx; i < max_depth; i++) {
        if (!free_if_required(i)) {
            // Return if moves[i] is NULL as all other moves will be NULL
            return;
        }
    }
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
    // Add the node to the cache unconditionally
    // This prevents the solver from entering loops
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
    Move* cheat_step_one = malloc(sizeof(Move));
    Move* cheat_step_two = malloc(sizeof(Move));
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
                found_solution = true;
                free_moves_from(depth);
                moves[depth] = copy_move(move);
                // If the timer is not already running, start it
                start(timer, continue_millis);
                max_moves = depth + 1;
                unapply_move(board, move);
                goto finalise;
            }
            int found = step(board, depth + 1, max_moves, allow_cheat);
            if (found != -1) {
                // Found a solution in fewer moves than before
                found_solution = true;
                free_if_required(depth);
                moves[depth] = copy_move(move);
                max_moves = found;
                // If the timer is not already running, start it
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
                const int two_move_col = two_move_cheat_col;
                const bool is_two_move = two_move_col != -1;
                if (is_two_move) {
                    // Set up the two steps using the data from move and two_move_col
                    cheat_step_one->from_x = two_move_col;
                    cheat_step_one->from_y = board->cols[two_move_col]->count - 1;
                    cheat_step_one->to_x = move->to_x;
                    cheat_step_one->to_y = move->to_y;
                    cheat_step_one->is_cheat = true;
                    cheat_step_two->from_x = move->from_x;
                    cheat_step_two->from_y = move->from_y;
                    cheat_step_two->to_x = two_move_col;
                    cheat_step_two->to_y = board->cols[two_move_col]->count - 1;
                    cheat_step_two->is_cheat = false;

                    // Remember that this is a two-step Cheat in order to
                    // finalise correctly
                    depth++;
                }
                apply_move(board, move);
                if (solved(board)) {
                    // Solving the board this move is the best I can do; start a timer
                    // and return
                    found_solution = true;
                    free_moves_from(depth);
                    if (is_two_move) {
                        free_if_required(depth - 1);
                        moves[depth - 1] = copy_move(cheat_step_one);
                        moves[depth] = copy_move(cheat_step_two);
                    } else {
                        moves[depth] = copy_move(move);
                    }
                    // If the timer is not already running, start it
                    start(timer, continue_millis);
                    max_moves = depth + 1;
                    unapply_move(board, move);
                    goto finalise;
                }
                int found = step(board, depth + 1, max_moves, allow_cheat);
                if (found != -1) {
                    // Found a solution in fewer moves than before
                    found_solution = true;
                    free_if_required(depth);
                    if (is_two_move) {
                        free_if_required(depth - 1);
                        moves[depth - 1] = copy_move(cheat_step_one);
                        moves[depth] = copy_move(cheat_step_two);
                    } else {
                        moves[depth] = copy_move(move);
                        // Found a solution better than any found double-Cheat
                        // so remember that this ISN'T a double-Cheat
                    }
                    max_moves = found;
                    // If the timer is not already running, start it
                    start(timer, continue_millis);
                }
                unapply_move(board, move);
                if (is_two_move) {
                    // Decrement depth to return to the correct place
                    depth--;
                }
            }
        }
    }
finalise:
    // Cut the node out if I had to allocate it and we're not within the minimum cache
    // boundary, or if I've proven that it's possible to solve the board
    if (found_solution
        || (free_node && cache_boundary != -1 && remaining_moves > cache_boundary)) {
        if (last_node) {
            last_node->next = node->next;
        } else {
            cache[board_hash] = (BoardHashTable_LLNode*)node->next;
        }
        free((void*)node->board_state);
        free(node);
    }
    // Free the loop variables to avoid a memory leak
    free(move);
    free(cheat_step_one);
    free(cheat_step_two);
    // If I found a solution, return the number of moves it took
    if (found_solution) {
        return max_moves;
    }
    return -1;
}

int main(int argc, string argv[]) {
    extern string optarg;
    extern int optind, optopt;
    bool solver_allow_cheat = false;

    int opt;
    int exit_code = EXIT_FAILURE;
    while ((opt = getopt(argc, argv, ":chn:t:m:")) != -1) {
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
            case '?':
                eprintfln("%s: -%c: invalid option", argv[0], optopt);
                goto print_help;
            case ':':
                eprintfln("%s: -%c: option requires an argument", argv[0], optopt);
                goto print_help;
            case 'h':
                exit_code = EXIT_SUCCESS;
                goto print_help;  // this line suppresses -Werror=implicit-fallthrough
            default:
            print_help:
                eprintfln(
                    "Usage: %s [-ch] [-n <moves>=16] [-t <milliseconds>=500] [-m <max "
                    "moves>=1024] <STATE>",
                    argv[0]);
                if (exit_code == EXIT_SUCCESS) {
                    eprintln("  -c                  Allow Cheating.");
                    eprintln(
                        "                      The solver will only attempt to Cheat "
                        "if a solution cannot be found without Cheating.");
                    eprintln("  -h                  Print this message and exit.");
                    eprintln(
                        "  -n <cache boundary> The maximum number of moves remaining "
                        "for the board state to be cached.");
                    eprintln("                      Defaults to 16.");
                    eprintln("                      If set to -1, all board states are "
                             "cached. (Not recommended).");
                    eprintln("  -t <milliseconds>   How long to continue searching for "
                             "a more optimal solution after one is found.");
                    eprintln("                      Defaults to 500 (0.5 seconds).");
                    eprintln(
                        "                      If set to -1, the solver will search "
                        "all possibilities to find the most optimal solution.");
                    eprintln(
                        "  -m <maximum moves>  The maximum number of moves to allow.");
                    eprintln("                      Defaults to 1024.");
                    eprintln("                      Very large values will cause "
                             "allocation errors.");
                    eprintln("                      It is recommended to make this a "
                             "power of two as the solver will only attempt to solve "
                             "with maximum depths of powers of two.");
                }
                exit(exit_code);
        }
    }
    if (!argv[optind]) {
        println("Need starting board state");
        exit(EXIT_FAILURE);
    }
    Board* board = parse_input(argv[optind]);

    int start_max_depth = min(64, max_depth);
    moves = calloc(max_depth, sizeof(Move*));

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
