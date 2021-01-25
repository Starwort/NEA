#include "debug.h"

void validate_board(const Board* board) {
    int found[15] = {0};
    string err;
    for (int i = 0; i < 6; i++) {
        Column* col_to_check = board->cols[i];
        if (col_to_check->count > 15) {
            err = "Column count too large (> 15)";
            goto invalid;
        }
        if (col_to_check->cheated
            && col_to_check->stack_begin != col_to_check->count - 1) {
            err = "Cheated column has stack_begin != count - 1";
            goto invalid;
        }
        if (stack_begin(col_to_check) != col_to_check->stack_begin) {
            err = "Column stack_begin does not match computed stack_begin";
            goto invalid;
        }
        for (int y = 0; y < col_to_check->count; y++) {
            found[col_to_check->cards[y]]++;
        }
    }
    for (int c = 6; c < 15; c++) {
        if (found[c] < 4) {
            err = "Card X appeared too few times";
            switch (c) {
                case 6:
                    err[5] = '6';
                    break;
                case 7:
                    err[5] = '7';
                    break;
                case 8:
                    err[5] = '8';
                    break;
                case 9:
                    err[5] = '9';
                    break;
                case 10:
                    err[5] = '0';
                    break;
                case 11:
                    err[5] = 'V';
                    break;
                case 12:
                    err[5] = 'D';
                    break;
                case 13:
                    err[5] = 'K';
                    break;
                case 14:
                    err[5] = 'T';
                    break;
            }
            goto invalid;
        } else if (found[c] > 4) {
            err = "Card X appeared too many times";
            switch (c) {
                case 6:
                    err[5] = '6';
                    break;
                case 7:
                    err[5] = '7';
                    break;
                case 8:
                    err[5] = '8';
                    break;
                case 9:
                    err[5] = '9';
                    break;
                case 10:
                    err[5] = '0';
                    break;
                case 11:
                    err[5] = 'V';
                    break;
                case 12:
                    err[5] = 'D';
                    break;
                case 13:
                    err[5] = 'K';
                    break;
                case 14:
                    err[5] = 'T';
                    break;
            }
            goto invalid;
        } else {
            continue;
        }
    }
    return;
invalid:
    eprintln("ERROR: Board state is invalid");
    eprintfln("       %s", err);
}

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
