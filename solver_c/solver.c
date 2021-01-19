#include "solver.h"

Move* moves[1024];

/* Search recursively for the next move; backtracking.
 * Return -1 if no valid moves found, non-negative to
 * indicate number of moves.
 *
 * Cheating should now be implemented
 */
int step(Board* board, int depth, int max_moves, bool allow_cheat) {
    if (depth == max_moves) {
        return -1;
    }
    Move* move = malloc(sizeof(Move));
    Move* cheat_step_one;
    Move* cheat_step_two;
    int two_move_cheat_col;
    for (move->from_x = 0; move->from_x < 6; move->from_x++) {
        for (move->to_x = 0; move->to_x < 6; move->to_x++) {
            if (move->from_x == move->to_x) {
                continue;
            }
            if (board->cols[move->from_x]->count == 0) {
                continue;
            }
            move->from_y = board->cols[move->from_x]->stack_begin;
            move->to_y = board->cols[move->to_x]->count;
            moves[depth] = move;
            if (!can_move(board, move)) {
                continue;
            }
            if (move->is_cheat) {
                if (!allow_cheat) {
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
            } else {
                two_move_cheat_col = -1;
            }
            apply_move(board, move);
            if (solved(board)) {
                return depth + 1;
            }
            int found = step(board, depth + 1, max_moves, allow_cheat);
            if (found != -1) {
                return found;
            }
            unapply_move(board, move);
            if (move->is_cheat && two_move_cheat_col != -1) {
                depth--;
                free(cheat_step_one);
                free(cheat_step_two);
            }
        }
    }
    free(move);
    return -1;
}

int main(int argc, string argv[]) {
    if (argc < 2) {
        println("Need starting board state");
        return 1;
    }
    Board* board = parse_input(argv[1]);
    bool solver_allow_cheat = false;

    int opt;
    while ((opt = getopt(argc, argv, "c")) != -1) {
        switch (opt) {
            case 'c':
                solver_allow_cheat = true;
                break;
            default:
                eprintfln("Usage: %s [-c] <STATE>", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    for (int max_depth = 64; max_depth <= 1024; max_depth <<= 1) {
        int n_moves = step(board, 0, max_depth, false);
        if (n_moves == -1) {
            eprintfln("No solution found in %d moves", max_depth);
        } else {
            for (int i = 0; i < n_moves; i++) {
                printfln(
                    "%d %d %c> %d %d",
                    moves[i]->from_x,
                    moves[i]->from_y,
                    moves[i]->is_cheat ? '!' : '-',
                    moves[i]->to_x,
                    moves[i]->to_y);
            }
            return 0;
        }
    }
    if (solver_allow_cheat) {
        for (int max_depth = 64; max_depth <= 1024; max_depth <<= 1) {
            int n_moves = step(board, 0, max_depth, true);
            if (n_moves == -1) {
                eprintfln("No solution found in %d moves (Cheating)", max_depth);
            } else {
                for (int i = 0; i < n_moves; i++) {
                    printfln(
                        "%d %d %c> %d %d",
                        moves[i]->from_x,
                        moves[i]->from_y,
                        moves[i]->is_cheat ? '!' : '-',
                        moves[i]->to_x,
                        moves[i]->to_y);
                }
                return 0;
            }
        }
    }
    return 0;
}
