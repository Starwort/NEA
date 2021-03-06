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
            eprintfln("Bad input string: Expected 42 characters but only got %d", i);
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
    int num_cards_found = 0;
    int cards_found[15] = {0};
    for (int i = 0; i < 6; i++) {
        Column* this_column = board->cols[i];
        uint8 current_card;
        uint8 y = 0;
        while ((current_card = given_state[card++]) != '.' && current_card != '!') {
            uint8 card_char = current_card;
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
                        "Bad input string: Expected a digit in the range 6-9, '0', "
                        "'T', 'K', 'D', V, '!', or '.'; got '%c'",
                        current_card);
                    free_board(board);
                    exit(EXIT_FAILURE);
            }
            cards_found[current_card]++;
            if (cards_found[current_card] > 4) {
                eprintfln(
                    "Bad input string: Impossible state; Found %d %c (expected 4)",
                    cards_found[current_card],
                    card_char);
                free_board(board);
                exit(EXIT_FAILURE);
            }
            if (y > 14) {
                eprintfln(
                    "Bad input string: Impossible state; Found %d cards in column (max "
                    "15)",
                    y);
                free_board(board);
                exit(EXIT_FAILURE);
            }
            this_column->cards[y++] = current_card;
            num_cards_found++;
        }
        this_column->count = y;
        this_column->cheated = current_card == '!';
        this_column->stack_begin = stack_begin(this_column);
    }
    if (num_cards_found != 36) {
        eprintfln("Bad input string: Expected 36 cards, got %d", num_cards_found);
        free_board(board);
        exit(EXIT_FAILURE);
    }
    return board;
}
