#include "debug.h"

void visualise_board(Board* board, bool print_all) {
    int max_height = 0;
    for (int i = 0; i < 6; i++) {
        int tmp;
        if ((tmp = board->cols[i]->count) > max_height) {
            max_height = tmp;
        }
    }
    for (int y = 0; y < max_height; y++) {
        for (int x = 0; x < 6; x++) {
            Column* col = board->cols[x];
            if (col->cheated && y == col->count - 1) {
                printf("!%02d!", col->cards[y]);
            } else if (y == col->count) {
                if (print_all) {
                    printf("^%02d^", col->cards[y]);
                } else {
                    print("^^^^");
                }
            } else if (y == col->stack_begin) {
                printf(">%02d<", col->cards[y]);
            } else if (!print_all && y > col->count) {
                print("    ");
            } else {
                printf(" %02d ", col->cards[y]);
            }
        }
        println();
    }
}
