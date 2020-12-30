#include "compress.h"

/* Compress a board state; encode the board state as an array of uint16s
 *
 * Output array format: [count + cheat, card...]
 * count + cheat = cheat << 15 | count
 * Return array needs freeing!!
 */
uint16* compress(Board* board) {
    uint8 elems = 6;
    for (int i = 0; i < 6; i++) {
        elems += board->cols[i]->count;
    }
    uint16* out = malloc(elems * sizeof(uint16));
    uint8 idx = 0;
    for (int col = 0; col < 6; col++) {
        Column* column = board->cols[col];
        out[idx++] = column->count | ((uint16)column->cheated << 15);
        for (int card = 0; card < column->count; card++) {
            out[idx++] = column->cards[card];
        }
    }
    return out;
}