#include "test.h"
// This file is used for testing various components
// Current test: compression and comparison
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
    for (int i = 1; i < argc; i++) {
        Board* board = parse_input(argv[i]);
        string compressed = compress(board);
        printfln("Original:      %s", argv[i]);
        printfln("Compressed:    %s", compressed);
        printfln("Identical:     %s", !strcmp(argv[i], compressed) ? "true" : "false");
        printfln("Compare equal: %s", compare(board, compressed) ? "true" : "false");
    }
    return 0;
}