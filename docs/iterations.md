# Iterations

## Minimum Viable Product [2021-01-03T14:42Z]

- This version of the solver was very minimalistic, with no configuration, no error handling or input validation, and no Cheating mechanics (not required for many games, so is a viable product)
- As a result of its simplicity it is much simpler than each subsequent version
- Code:
  - <details><summary><code>board_info.c</code> (click to expand)</summary>

    ```c
      #include "board_info.h"

    bool legal_stack(Column* col, uint8 position) {
        uint8 value = col->cards[position];
        for (uint8 y = position; y < col->count; y++) {
            if (col->cards[y] != value--) {
                return false;
            }
        }
        return true;
    }

    uint8 stack_begin(Column* col) {
        if (col->count == 0) {
            return 0;
        }
        if (col->cheated) {
            return col->count - 1;
        }
        uint8 y = col->count - 1;
        uint8 card = col->cards[y];
        while (col->cards[--y] == ++card && y != 255) {
        }
        return y + 1;
    }

    bool can_move(Board* board, uint8 from_col, uint8 from_y, uint8 to_col) {
        Column* fcol = board->cols[from_col];
        Column* tcol = board->cols[to_col];
        if (tcol->cheated) {
            return false;
        }
        if (from_y < fcol->count - 1 && !legal_stack(fcol, from_y)) {
            return false;
        }
        if (tcol->count == 0) {
            return true;
        }
        return (tcol->cards[tcol->count - 1] == fcol->cards[from_y] + 1);
    }

    /* Determine if the board is solved
     *
     * The board is solved if all columns are either empty or
     * contain a full stack starting from the top of the tableau
     * (col->cards == {14, 13, 12, 11, 10, 9, 8, 7, 6} and
     * therefore col->count == 9 and col->stack_begin == 0)
     */
    bool solved(Board* board) {
        for (int col = 0; col < 6; col++) {
            if (board->cols[col]->count == 0
                || (board->cols[col]->count == 9 && board->cols[col]->stack_begin == 0)) {
                continue;
            }
            return false;
        }
        return true;
    }

    /* Apply move to board.
     *
     * Does not check validity of move.
     */
    void apply_move(Board* board, Move* move) {
        Column* from_col = board->cols[move->from_x];
        Column* to_col = board->cols[move->to_x];
        uint8 n_elements = from_col->count - move->from_y;
        memcpy(to_col->cards + move->to_y, from_col->cards + move->from_y, n_elements);
        from_col->count = move->from_y;
        from_col->stack_begin = stack_begin(from_col);
        to_col->count += n_elements;
    }

    /* Unapply move to board; opposite of apply_move */
    void unapply_move(Board* board, Move* move) {
        Column* from_col = board->cols[move->from_x];
        Column* to_col = board->cols[move->to_x];
        uint8 n_elements = to_col->count - move->to_y;
        memcpy(from_col->cards + move->from_y, to_col->cards + move->to_y, n_elements);
        from_col->stack_begin = from_col->count;
        from_col->count = move->from_y + n_elements;
        to_col->count = move->to_y;
    }

    ```
    </details>
  - <details><summary><code>board_info.h</code> (click to expand)</summary>

    ```c
      #pragma once
    #include "types.h"

    bool legal_stack(Column* col, uint8 position);
    uint8 stack_begin(Column* col);
    bool can_move(Board* board, uint8 from_col, uint8 from_y, uint8 to_col);
    bool solved(Board* board);
    void apply_move(Board* board, Move* move);
    void unapply_move(Board* board, Move* move);

    ```
    </details>
  - <details><summary><code>compress.c</code> (click to expand)</summary>

    ```c
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
    ```
    </details>
  - <details><summary><code>compress.h</code> (click to expand)</summary>

    ```c
      #include "types.h"

    string compress(Board* board);
    bool compare(Board* board, string compressed);
    ```
    </details>
  - <details><summary><code>hash.c</code> (click to expand)</summary>

    ```c
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
    ```
    </details>
  - <details><summary><code>hash.h</code> (click to expand)</summary>

    ```c
      #include "types.h"

    uint32 powmod(uint32 y, uint32 x);
    uint32 hash(Board* board);
    uint32 hash_column(Column* column);
    ```
    </details>
  - <details><summary><code>memory.c</code> (click to expand)</summary>

    ```c
      #include "memory.h"

    /* Allocate a board and its columns
     * Needs freeing after use
     */
    Board* allocate_board() {
        Board* board = malloc(sizeof(Board));
        board->depth = 0;
        for (int i = 0; i < 6; i++) {
            board->cols[i] = malloc(sizeof(Column));
        }
        return board;
    }
    /* Free a board and its columns
     */
    void free_board(Board* board) {
        for (int i = 0; i < 6; i++) {
            free(board->cols[i]);
        }
        free(board);
    }
    ```
    </details>
  - <details><summary><code>memory.h</code> (click to expand)</summary>

    ```c
      #include "types.h"

    Board* allocate_board();
    void free_board(Board* board);
    ```
    </details>
  - <details><summary><code>parse.c</code> (click to expand)</summary>

    ```c
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
     */
    Board* parse_input(string given_state) {
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
    ```
    </details>
  - <details><summary><code>parse.h</code> (click to expand)</summary>

    ```c
      #pragma once
    #include "board_info.h"
    #include "memory.h"
    #include "types.h"

    Board* parse_input(string given_state);
    ```
    </details>
  - <details><summary><code>solver.c</code> (click to expand)</summary>

    ```c
      #include "solver.h"

    Move* moves[1024];

    /* Search recursively for the next move; backtracking.
     * Return -1 if no valid moves found, non-negative to
     * indicate number of moves.
     *
     * TODO: implement cheating
     */
    int step(Board* board, int depth, int max_moves) {
        printfln("Depth %d:", depth);
        visualise_board(board, false);
        Move* move = malloc(sizeof(Move));
        if (depth == max_moves) {
            return -1;
        }
        for (move->from_x = 0; move->from_x < 6; move->from_x++) {
            for (move->to_x = 0; move->to_x < 6; move->to_x++) {
                if (move->from_x == move->to_x) {
                    continue;
                }
                if (board->cols[move->from_x]->count == 0) {
                    continue;
                }
                move->from_y = board->cols[move->from_x]->stack_begin;
                move->to_y = board->cols[move->to_x]->count;
                moves[depth] = move;
                if (!can_move(board, move->from_x, move->from_y, move->to_x)) {
                    continue;
                }
                apply_move(board, move);
                if (solved(board)) {
                    return depth + 1;
                }
                int found = step(board, depth + 1, max_moves);
                if (found != -1) {
                    return found;
                }
                unapply_move(board, move);
            }
        }
        free(move);
        return -1;
    }

    int main(int argc, string argv[]) {
        if (argc < 2) {
            println("Need starting board state");
            return 1;
        }
        Board* board = parse_input(argv[1]);

        for (int max_depth = 64; max_depth <= 1024; max_depth <<= 1) {
            int n_moves = step(board, 0, max_depth);
            if (n_moves == -1) {
                eprintfln("No solution found in %d moves", max_depth);
            } else {
                for (int i = 0; i < n_moves; i++) {
                    printfln(
                        "%d %d -> %d %d",
                        moves[i]->from_x,
                        moves[i]->from_y,
                        moves[i]->to_x,
                        moves[i]->to_y);
                }
                return 0;
            }
        }
        return 0;
    }
    ```
    </details>
  - <details><summary><code>solver.h</code> (click to expand)</summary>

    ```c
      #pragma once
    #include "board_info.h"
    #include "debug.h"
    #include "parse.h"
    #include "types.h"

    int step(Board* board, int depth, int max_moves);
    ```
    </details>
  - <details><summary><code>types.h</code> (click to expand)</summary>

    ```c
      #pragma once
    #include "common.h"

    typedef uint8 Card;

    typedef struct _Column {
        Card cards[14];
        uint8 count;
        uint8 stack_begin;
        bool cheated;
    } Column;

    typedef struct _Board {
        Column* cols[6];  // Board.cols[x][y]
        int depth;
    } Board;

    typedef struct _Move {
        uint8 from_x;
        uint8 from_y;
        uint8 to_x;
        uint8 to_y;
        bool is_cheat;
    } Move;
    ```
    </details>

## Add Cheating [2021-01-19T19:00Z]

- This version of the solver added support for Cheating, as well as experimental support for options (using the `argp` library, which was later dropped in favour of the cross-platform `getopt` module).
- Additionally, several algorithm and style improvements were made, including preparation for the use of a hash table to optimise solution search, along with documentation, and adding trailing blank lines to every source file
- Code:
  - <details><summary><code>board_info.c</code> (click to expand)</summary>

    ```c
      #include "board_info.h"

    /* Determine if a stack is legal
     */
    bool legal_stack(Column* col, uint8 position) {
        uint8 value = col->cards[position];
        for (uint8 y = position; y < col->count; y++) {
            if (col->cards[y] != value--) {
                return false;
            }
        }
        return true;
    }

    /* Determine where the stack begins
     */
    uint8 stack_begin(Column* col) {
        // if there are no cards in the column then there is no stack
        if (col->count == 0) {
            return 0;
        }
        // if there is a cheated card then that is the entire stack
        if (col->cheated) {
            return col->count - 1;
        }
        uint8 y = col->count - 1;
        uint8 card = col->cards[y];
        while (--y != 255 && col->cards[y] == ++card) {
        }
        return y + 1;
    }

    /* Check if a given move is legal on a given board
     * Sets move->is_cheat
     */
    bool can_move(Board* board, Move* move) {
        int from_col = move->from_x;
        int from_y = move->from_y;
        int to_col = move->to_x;
        Column* fcol = board->cols[from_col];
        Column* tcol = board->cols[to_col];

        // cards can never be moved onto a column which contains a Cheated card
        if (tcol->cheated) {
            return false;
        }

        // never move from the beginning of a stack to an empty stack
        if (fcol->stack_begin == 0 && tcol->count == 0) {
            return false;
        }

        // never move an invalid stack - this could actually be omitted as it's
        // impossible for such a Move* to be generated
        // if (from_y < fcol->count - 1 && !legal_stack(fcol, from_y)) {
        //     return false;
        // }

        move->is_cheat = false;  // assume not a cheat until we determine it is
        // if there are no cards in this column then the move is valid
        if (tcol->count == 0) {
            return true;
        }

        // otherwise check the top card and compare it to the card being moved
        if (tcol->cards[tcol->count - 1] == fcol->cards[from_y] + 1) {
            return true;
        }

        // if we reach here then the attempted move Cheats
        move->is_cheat = true;
        if (fcol->cheated) {
            // this is only a valid move if fcol->cards[fcol->count - 1] can
            // also be found in a legitimate position on a column that is neither
            // fcol nor tcol
            for (int i = 0; i < 6; i++) {
                if (i == from_col || i == to_col) {
                    continue;
                }
                Column* col = board->cols[i];
                // the stack must be more than one card (the card must be
                // in a valid position)
                if (col->stack_begin < col->count - 1
                    // and must end with the card we're trying to move
                    && col->cards[col->count - 1] == fcol->cards[fcol->count - 1]) {
                    return true;  // legal move (that is technically 2 moves)
                }
            }
            // not legal; no column permits this move
            return false;
        }
        // Cheating with a card that was previously not Cheated is always valid
        return true;
    }

    /* Determine if move is the special two-move operation Cheat -> Cheat
     * If it is, return the column of the intermediate column.
     * Else, return -1
     *
     * Assume that move is a legal Cheat move
     */
    int column_if_two_moves(Board* board, Move* move) {
        int from_col = move->from_x;
        int from_y = move->from_y;
        int to_col = move->to_x;
        Column* fcol = board->cols[from_col];
        Column* tcol = board->cols[to_col];
        if (fcol->cheated) {
            // find the column using the algorithm from can_move
            for (int i = 0; i < 6; i++) {
                if (i == from_col || i == to_col) {
                    continue;
                }
                Column* col = board->cols[i];
                if (col->stack_begin < col->count - 1
                    && col->cards[col->count - 1] == fcol->cards[fcol->count - 1]) {
                    return i;  // found the two-move column
                }
            }
        }
        return -1;  // this move cannot be Cheat -> Cheat
    }

    /* Determine if the board is solved
     *
     * The board is solved if all columns are either empty or
     * contain a full stack starting from the top of the tableau
     * (col->cards == {14, 13, 12, 11, 10, 9, 8, 7, 6} and
     * therefore col->count == 9 and col->stack_begin == 0)
     */
    bool solved(Board* board) {
        for (int col = 0; col < 6; col++) {
            if (board->cols[col]->count == 0  // column is empty
                || (board->cols[col]->count == 9  // or contains the full stack
                    && board->cols[col]->stack_begin == 0)) {
                continue;
            }
            return false;
        }
        return true;
    }

    /* Apply move to board.
     *
     * Does not check validity of move.
     */
    void apply_move(Board* board, Move* move) {
        Column* from_col = board->cols[move->from_x];
        Column* to_col = board->cols[move->to_x];
        uint8 n_elements = from_col->count - move->from_y;
        memcpy(to_col->cards + move->to_y, from_col->cards + move->from_y, n_elements);
        from_col->count = move->from_y;
        from_col->stack_begin = stack_begin(from_col);
        to_col->count += n_elements;
    }

    /* Unapply move to board; opposite of apply_move
     */
    void unapply_move(Board* board, Move* move) {
        Column* from_col = board->cols[move->from_x];
        Column* to_col = board->cols[move->to_x];
        uint8 n_elements = to_col->count - move->to_y;
        memcpy(from_col->cards + move->from_y, to_col->cards + move->to_y, n_elements);
        from_col->stack_begin = from_col->count;
        from_col->count = move->from_y + n_elements;
        to_col->count = move->to_y;
    }

    ```
    </details>
  - <details><summary><code>board_info.h</code> (click to expand)</summary>

    ```c
      #pragma once
    #include "types.h"

    bool legal_stack(Column* col, uint8 position);
    uint8 stack_begin(Column* col);
    bool can_move(Board* board, Move* move);
    int column_if_two_moves(Board* board, Move* move);
    bool solved(Board* board);
    void apply_move(Board* board, Move* move);
    void unapply_move(Board* board, Move* move);

    ```
    </details>
  - <details><summary><code>compress.c</code> (click to expand)</summary>

    ```c
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

    ```
    </details>
  - <details><summary><code>compress.h</code> (click to expand)</summary>

    ```c
      #include "types.h"

    string compress(Board* board);
    bool compare(Board* board, string compressed);

    ```
    </details>
  - <details><summary><code>hash.c</code> (click to expand)</summary>

    ```c
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

    ```
    </details>
  - <details><summary><code>hash.h</code> (click to expand)</summary>

    ```c
      #include "types.h"

    uint32 powmod(uint32 y, uint32 x);
    uint32 hash(Board* board);
    uint32 hash_column(Column* column);

    ```
    </details>
  - <details><summary><code>memory.c</code> (click to expand)</summary>

    ```c
      #include "memory.h"

    /* Allocate a board and its columns
     * Needs freeing after use
     */
    Board* allocate_board() {
        Board* board = malloc(sizeof(Board));
        board->depth = 0;
        for (int i = 0; i < 6; i++) {
            board->cols[i] = malloc(sizeof(Column));
        }
        return board;
    }

    /* Free a board and its columns
     */
    void free_board(Board* board) {
        for (int i = 0; i < 6; i++) {
            free(board->cols[i]);
        }
        free(board);
    }

    /* Allocate a BoardHashTable_LLNode
     */
    BoardHashTable_LLNode* create_node(string board_state, int depth) {
        BoardHashTable_LLNode* node = malloc(sizeof(BoardHashTable_LLNode));
        node->board_state = board_state;
        node->unsolvable_in = depth;
        node->next = NULL;
        return node;
    }

    ```
    </details>
  - <details><summary><code>memory.h</code> (click to expand)</summary>

    ```c
      #include "types.h"

    Board* allocate_board();
    void free_board(Board* board);
    BoardHashTable_LLNode* create_node(string board_state, int depth);

    ```
    </details>
  - <details><summary><code>parse.c</code> (click to expand)</summary>

    ```c
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
    Board* parse_input(string given_state) {
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

    ```
    </details>
  - <details><summary><code>parse.h</code> (click to expand)</summary>

    ```c
      #pragma once
    #include "board_info.h"
    #include "memory.h"
    #include "types.h"

    Board* parse_input(string given_state);

    ```
    </details>
  - <details><summary><code>solver.c</code> (click to expand)</summary>

    ```c
      #include "solver.h"

    Move* moves[1024];

    /* Search recursively for the next move; backtracking.
     * Return -1 if no valid moves found, non-negative to
     * indicate number of moves.
     *
     * Cheating should now be implemented
     */
    int step(Board* board, int depth, int max_moves, bool allow_cheat) {
        if (depth == max_moves) {
            return -1;
        }
        Move* move = malloc(sizeof(Move));
        Move* cheat_step_one;
        Move* cheat_step_two;
        int two_move_cheat_col;
        for (move->from_x = 0; move->from_x < 6; move->from_x++) {
            for (move->to_x = 0; move->to_x < 6; move->to_x++) {
                if (move->from_x == move->to_x) {
                    continue;
                }
                if (board->cols[move->from_x]->count == 0) {
                    continue;
                }
                move->from_y = board->cols[move->from_x]->stack_begin;
                move->to_y = board->cols[move->to_x]->count;
                moves[depth] = move;
                if (!can_move(board, move)) {
                    continue;
                }
                if (move->is_cheat) {
                    if (!allow_cheat) {
                        continue;
                    }
                    two_move_cheat_col = column_if_two_moves(board, move);
                    if (two_move_cheat_col != -1) {
                        cheat_step_one = malloc(sizeof(Move));
                        cheat_step_two = malloc(sizeof(Move));
                        cheat_step_one->from_x = two_move_cheat_col;
                        cheat_step_one->from_y = board->cols[two_move_cheat_col]->count - 1;
                        cheat_step_one->to_x = move->to_x;
                        cheat_step_one->to_y = move->to_y;
                        cheat_step_one->is_cheat = true;
                        cheat_step_two->from_x = move->from_x;
                        cheat_step_two->from_y = move->from_y;
                        cheat_step_two->to_x = two_move_cheat_col;
                        cheat_step_two->to_y = board->cols[two_move_cheat_col]->count - 1;
                        cheat_step_two->is_cheat = false;
                        moves[depth++] = cheat_step_one;
                        moves[depth] = cheat_step_two;
                    }
                } else {
                    two_move_cheat_col = -1;
                }
                apply_move(board, move);
                if (solved(board)) {
                    return depth + 1;
                }
                int found = step(board, depth + 1, max_moves, allow_cheat);
                if (found != -1) {
                    return found;
                }
                unapply_move(board, move);
                if (move->is_cheat && two_move_cheat_col != -1) {
                    depth--;
                    free(cheat_step_one);
                    free(cheat_step_two);
                }
            }
        }
        free(move);
        return -1;
    }

    int main(int argc, string argv[]) {
        if (argc < 2) {
            println("Need starting board state");
            return 1;
        }
        Board* board = parse_input(argv[1]);
        struct arguments arguments;

        arguments.cheat = false;
        argp_parse(&argp, argc, argv, 0, 0, &arguments);

        for (int max_depth = 64; max_depth <= 1024; max_depth <<= 1) {
            int n_moves = step(board, 0, max_depth, false);
            if (n_moves == -1) {
                eprintfln("No solution found in %d moves", max_depth);
            } else {
                for (int i = 0; i < n_moves; i++) {
                    printfln(
                        "%d %d %c> %d %d",
                        moves[i]->from_x,
                        moves[i]->from_y,
                        moves[i]->is_cheat ? '!' : '-',
                        moves[i]->to_x,
                        moves[i]->to_y);
                }
                return 0;
            }
        }
        if (arguments.cheat) {
            for (int max_depth = 64; max_depth <= 1024; max_depth <<= 1) {
                int n_moves = step(board, 0, max_depth, true);
                if (n_moves == -1) {
                    eprintfln("No solution found in %d moves (Cheating)", max_depth);
                } else {
                    for (int i = 0; i < n_moves; i++) {
                        printfln(
                            "%d %d %c> %d %d",
                            moves[i]->from_x,
                            moves[i]->from_y,
                            moves[i]->is_cheat ? '!' : '-',
                            moves[i]->to_x,
                            moves[i]->to_y);
                    }
                    return 0;
                }
            }
        }
        return 0;
    }

    ```
    </details>
  - <details><summary><code>solver.h</code> (click to expand)</summary>

    ```c
      #pragma once
    #include "board_info.h"
    // #include "debug.h"
    #include "parse.h"
    #include "types.h"

    #include <argp.h>

    const char* argp_program_version = "molek-syntez-solitaire-solver";
    const char* argp_program_bug_address = "";
    static char doc[] = "Solver for MOLEK-SYNTEZ solitaire minigame";
    static char args_doc[] = "<STATE>";
    static struct argp_option options[] = {
        {"cheat",
         'c',
         0,
         0,
         "Allow Cheating (only takes effect if "
         "no solution is found without Cheating)"},
        {0}};

    struct arguments {
        bool cheat;
    };

    static error_t parse_opt(int key, char* arg, struct argp_state* state) {
        struct arguments* arguments = state->input;
        switch (key) {
            case 'c':
                arguments->cheat = true;
                break;
            case ARGP_KEY_ARG:
                return 0;
            default:
                return ARGP_ERR_UNKNOWN;
        }
        return 0;
    }

    static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

    int step(Board* board, int depth, int max_moves, bool allow_cheat);

    ```
    </details>
  - <details><summary><code>types.h</code> (click to expand)</summary>

    ```c
      #pragma once
    #include "common.h"

    typedef uint8 Card;

    typedef struct _Column {
        Card cards[14];
        uint8 count;
        uint8 stack_begin;
        bool cheated;
    } Column;

    typedef struct _Board {
        Column* cols[6];  // Board.cols[x][y]
        int depth;
    } Board;

    typedef struct _Move {
        uint8 from_x;
        uint8 from_y;
        uint8 to_x;
        uint8 to_y;
        bool is_cheat;
    } Move;

    typedef struct _BoardHashTable_LLNode {
        string board_state;
        int unsolvable_in;
        struct _BoardHashTable_LLNode* next;
    } BoardHashTable_LLNode;

    ```
    </details>
