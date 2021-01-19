#include "solver.h"

Move* moves[1024];

/* Search recursively for the next move; backtracking.
 * Return -1 if no valid moves found, non-negative to
 * indicate number of moves.
 *
 * TODO: implement cheating
 */
int step(Board* board, int depth, int max_moves) {
    // printfln("Depth %d:", depth);
    // visualise_board(board, false);
    Move* move = malloc(sizeof(Move));
    if (depth == max_moves) {
        return -1;
    }
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
            apply_move(board, move);
            if (solved(board)) {
                return depth + 1;
            }
            int found = step(board, depth + 1, max_moves);
            if (found != -1) {
                return found;
            }
            unapply_move(board, move);
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

    for (int max_depth = 64; max_depth <= 1024; max_depth <<= 1) {
        int n_moves = step(board, 0, max_depth);
        if (n_moves == -1) {
            eprintfln("No solution found in %d moves", max_depth);
        } else {
            for (int i = 0; i < n_moves; i++) {
                printfln(
                    "%d %d -> %d %d",
                    moves[i]->from_x,
                    moves[i]->from_y,
                    moves[i]->to_x,
                    moves[i]->to_y);
            }
            return 0;
        }
    }
    return 0;
}
