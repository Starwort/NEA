#include "hash.h"

/* The MODULUS and GENERATOR are used in the hash function to generate a
 * pseudo-random number based on each board position. The MODULUS is a
 * prime number with enough bits to fill the hash table, and the GENERATOR
 * is a primitive root with respect to that modulus. Conveniently, 2^31-1
 * is a prime number, and the generator here is that used in the MINSTD
 * Lehmer RNG and is equal to 7^5.
 */
#define MODULUS 2147483647
#define GENERATOR 16807

/* The powmod function calculates (y^x) mod (MODULUS) and is used in the
 * hash function. We are assuming that uint32 is long enough to hold the
 * modulus and uint64 is long enough to hold two longs multiplied
 * together.
 *
 * Square-and-multiply method. Check each bit of x in turn, square the
 * answer so far and multiply by y if the bit was a 1.
 */
uint32 powmod(uint32 y, uint32 x) {
    uint32 answer = 1;
    int i;
    for (i = 0; i < 32; i++) {
        if (i > 0 && answer > 1) {
            answer = ((uint64)answer * (uint64)answer) % MODULUS;
        }
        if (x & 0x80000000) {
            answer = ((uint64)answer * (uint64)y) % MODULUS;
        }
        x <<= 1;
    }
    return answer;
}

/* The hash function calculates a hash based on the current board position.
 * The hash is a pseudo-random number which must be the same for any
 * two identical board positions. In this game of Solitaire we consider
 * two board positions similar if they contain the same columns of cards
 * in any order, so the hash function is invariant when columns are swapped.
 *
 * The value of the hash will be (((a^col_1)^col_2)^...)^col_N mod (MODULUS) and
 * then further modified by the cell card if any; where a is the RNG generator
 * and col_1...col_N are hash values computed from the card columns. This
 * value is equivalent to a^(col_1*col_2*...*col_N) and therefore the order of
 * the columns is not significant. Each column hash must not be zero, and
 * so will be 1 if the column is empty.
 */
uint32 hash(Board* board) {
    uint32 rv = GENERATOR;
    for (uint8 col = 0; col < 6; col++) {
        rv = powmod(rv, hash_column(board->cols[col]));
    }
    return rv;
}

/* The column hash function calculates a hash based on a column's values.
 * The hash is a pseudo-random number which must be the same for any two
 * identical columns. In this game of Solitaire we consider two columns
 * of cards similar if they contain the same cards in the same order and
 * have the same 'cheat' flag.
 *
 * The value of the hash will be:
 * (cheated << 31) | (((33+card_0)*33+card_1)*33+...)*33+card_N mod (MODULUS)
 * where card_n is equivalent to column->cards[n] and card_N = the last card
 * in the column.
 */
uint32 hash_column(Column* column) {
    uint32 rv = 1;
    for (uint8 card = 0; card < column->count; card++) {
        rv = ((uint64)rv * (uint64)33 + column->cards[card]) % MODULUS;
    }
    return rv | (uint32)(column->cheated << 31);
}
