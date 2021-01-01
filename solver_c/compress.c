#include "compress.h"

/* Compress a board state to a board state string
 * Return string needs freeing
 */
string compress(Board* board) {
    // board state string is always 42 characters: 36 cards + 6 columns
    string out = malloc(42 * sizeof(char));
    uint8 idx = 0;
    for (int col = 0; col < 6; col++) {
        Column* column = board->cols[col];
        for (int card = 0; card < column->count; card++) {
            switch (column->cards[card]) {
                case 14:
                    out[idx++] = 'T';
                    break;
                case 13:
                    out[idx++] = 'K';
                    break;
                case 12:
                    out[idx++] = 'D';
                    break;
                case 11:
                    out[idx++] = 'V';
                    break;
                case 10:
                    out[idx++] = '0';
                    break;
                default:
                    out[idx++] = column->cards[card] + '0';
                    break;
            }
        }
        out[idx++] = column->cheated ? '!' : '.';
    }
    return out;
}

bool _compare(Column* col, string* compressed) {
    if ((*compressed)[col->count] != (col->cheated ? '!' : '.')) {
        return false;
    }
    int card;
    for (card = 0; card < col->count; card++) {
        switch ((*compressed)[card]) {
            case '.':
            case '!':
                return false;
            case 'T':
                if (col->cards[card] != 14) {
                    return false;
                }
                break;
            case 'K':
                if (col->cards[card] != 13) {
                    return false;
                }
                break;
            case 'D':
                if (col->cards[card] != 12) {
                    return false;
                }
                break;
            case 'V':
                if (col->cards[card] != 11) {
                    return false;
                }
                break;
            case '0':
                if (col->cards[card] != 10) {
                    return false;
                }
                break;
            default:
                if (col->cards[card] != ((*compressed)[card] - '0')) {
                    return false;
                }
                break;
        }
    }
    *compressed += card + 1;
    return true;
}

/* Compare a board state to a compressed board string.
 * Column order does not matter and so is ignored.
 */
bool compare(Board* board, string compressed) {
    bool matched[6] = {0, 0, 0, 0, 0, 0};
    for (int col_compressed = 0; col_compressed < 6; col_compressed++) {
        bool found_match = false;
        for (int col_real = 0; col_real < 6; col_real++) {
            if (matched[col_real]) {
                continue;
            }
            if (_compare(board->cols[col_real], &compressed)) {
                matched[col_real] = found_match = true;
                break;
            }
        }
        if (!found_match) {
            return false;
        }
    }
    return true;
}