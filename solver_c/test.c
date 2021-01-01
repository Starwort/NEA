#include "test.h"
// This file is used for testing various components
// Current test: column order invariance
void visualise_board(Board* board) {
    for (int y = 0; y < 14; y++) {
        for (int x = 0; x < 6; x++) {
            Column* col = board->cols[x];
            if (col->cheated && y == col->count - 1) {
                printf("!%02d!", col->cards[y]);
            } else if (y == col->count) {
                printf("^%02d^", col->cards[y]);
            } else if (y == col->stack_begin) {
                printf(">%02d<", col->cards[y]);
            } else {
                printf(" %02d ", col->cards[y]);
            }
        }
        println();
    }
}

int main(int argc, string argv[]) {
    argv += 1;
    argc -= 1;
    if (argc % 2 != 0) {
        println("Need two board states to compare for each test");
        return 1;
    }
    for (int i = 0; i < argc;) {
        Board* board = parse_input(argv[i++]);
        string compressed = argv[i++];
        printfln("Comparing:     %s", compress(board));
        printfln("To:            %s", compressed);
        printfln("Compare equal: %s", compare(board, compressed) ? "true" : "false");
    }
    return 0;
}