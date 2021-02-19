#include "parse.h"

/* Given a board represented as a string, generate a board
 * object and populate its values.
 *
 * Board data is expected to be in the format
 * /([TKDV06-9]{0,15}(\.|!)){6}/ (given as a regex)
 *
 * T, K, D, V, 9, 8, 7, and 6 represent themselves, and 0
 * represents 10
 *
 * The board needs freeing after use.
 */
Board* parse_input(const string given_state) {
    int num_cols_found = 0;
    for (int i = 0; i < 42; i++) {
        if (given_state[i] == '\0') {
            eprintfln(
                "Unexpected end of string; Expected 42 characters but only got %d", i);
            exit(EXIT_FAILURE);
        }
        if (given_state[i] == '.' || given_state[i] == '!') {
            num_cols_found++;
        }
    }
    if (num_cols_found != 6) {
        eprintfln("Bad input string: Expected 6 columns, got %d", num_cols_found);
        exit(EXIT_FAILURE);
    }
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
                case '9':
                case '8':
                case '7':
                case '6':
                    current_card -= '0';
                    break;
                default:
                    eprintfln(
                        "Expected a digit in the range 6-9, '0', 'T', 'K', 'D', V, "
                        "'!', or '.'; got '%c'",
                        current_card);
                    exit(EXIT_FAILURE);
            }
            this_column->cards[y++] = current_card;
        }
        this_column->count = y;
        this_column->cheated = current_card == '!';
        this_column->stack_begin = stack_begin(this_column);
    }
    return board;
}
