#include "test.h"
// This file is used for testing various components
// Current test: parsing robustness
int main(int argc __attribute__((unused)), string argv[]) {
    Board* board = parse_input(argv[1]);
    free_board(board);
    return 0;
}
