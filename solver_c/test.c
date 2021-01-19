#include "test.h"
// This file is used for testing various components
// Current test: column order invariance (hashing)
int main(int argc, string argv[]) {
    for (int i = 1; i < argc; i++) {
        Board* board = parse_input(argv[i]);
        printfln("%d", hash(board));
    }
    return 0;
}
