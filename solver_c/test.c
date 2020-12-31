#include "test.h"
// This file is used for testing various components
// Current test: benchmark stack_begin (board_info.c)
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
    setlocale(LC_NUMERIC, "");
    if (argc < 2) {
        eprintln("Need a board state");
        return 1;
    }
    uint64 iterations = 10000000;
    if (argc >= 3) {
        iterations = atol(argv[2]);
        printfln(
            "Using %'lu iterations (instead of %'lu iterations)",
            iterations,
            10000000l);
    }
    Board* board = parse_input(argv[1]);
    Column* col = board->cols[0];
    struct timeval begin;
    gettimeofday(&begin, NULL);
    for (uint64 i = 0; i < iterations; i++) {
        stack_begin(col);
    }
    struct timeval end;
    gettimeofday(&end, NULL);
    time_t total_seconds = end.tv_sec - begin.tv_sec;
    if (end.tv_usec < begin.tv_usec) {
        end.tv_usec += 1000000;
        total_seconds -= 1;
    }
    printfln("After %'lu iterations:", iterations);
    suseconds_t total_micros = end.tv_usec - begin.tv_usec;
    printfln("Elapsed: %'lds %'ldµs", total_seconds, total_micros);
    float total_nanos = total_micros * 1000 + total_seconds * 1000000000;
    total_nanos /= iterations;
    printfln("Avg. time per iteration: %'fns", total_nanos);
    return 0;
}