#include "parse.h"

/* Given a board represented as a string, generate a board
 * object and populate its values.
 *
 * Board data is expected to be in the format
 * /([TKDV06-9]{0,14}(\.|!)){6}/ (given as a regex)
 *
 * T, K, D, V, 9, 8, 7, and 6 represent themselves, and 0
 * represents 10
 *
 * The board needs freeing after use.
 *
 * TODO: avoid segfault for bad input
 */
Board* parse_input(const string given_state) {
    Board* board = allocate_board();
    int card = 0;
    for (int i = 0; i < 6; i++) {
        Column* this_column = board->cols[i];
        uint8 current_card;
        uint8 y = 0;
        while ((current_card = given_state[card++]) != '.' && current_card != '!') {
            switch (current_card) {
                case 'T':
                    current_card = 14;
                    break;
                case 'K':
                    current_card = 13;
                    break;
                case 'D':
                    current_card = 12;
                    break;
                case 'V':
                    current_card = 11;
                    break;
                case '0':
                    current_card = 10;
                    break;
                default:
                    current_card -= '0';
            }
            this_column->cards[y++] = current_card;
        }
        this_column->count = y;
        this_column->cheated = current_card == '!';
        this_column->stack_begin = stack_begin(this_column);
    }
    return board;
}
