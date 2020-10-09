#include "common.h"

typedef struct _Card {
    int8 value;
    bool cheated;
} Card;

typedef struct _Board {
    Card cards[6][12];  // Board.cards[x][y]
    int depth;
} Board;

typedef struct _Move {
    int from_x;
    int from_y;
    int to_x;
    int to_y;
    bool is_cheat;
} Move;

int find_last_card_in_column(Card column[]) {
    for (int i = 0;; i++) {
        if (column[i].value == -1) {
            return i - 1;
        }
    }
}

void get_moves(Board board, Move out[]) {
    //
}

int main(int argc, string argv[]) {
    //
    return 0;
}