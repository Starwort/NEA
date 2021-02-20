# Iterations

## C solver

### Minimum Viable Product [2021-01-03T14:42Z]

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

### Add Cheating [2021-01-19T19:00Z]

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

### Input validation and bugfixes [2021-02-19T16:28Z]

- This version of the solver was created after multiple incremental improvements
- It adds a lot of input validation (specifically to the given board state), in addition to preventing the solver from performing some illegal moves (notably, attempting to Cheat cards onto complete stacks, which is an illegal move as when a column is completed it is locked)
- Code:
  - <details><summary><code>board_info.c</code> (click to expand)</summary>

    ```c
      #include "board_info.h"

    /* Determine if a stack is legal
     */
    bool legal_stack(const Column* col, uint8 position) {
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
    uint8 stack_begin(const Column* col) {
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
    bool can_move(const Board* board, Move* move) {
        const int from_col = move->from_x;
        const int from_y = move->from_y;
        const int to_col = move->to_x;
        const Column* fcol = board->cols[from_col];
        const Column* tcol = board->cols[to_col];

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

        if (from_y < fcol->count - 1) {
            // moving a stack; must not Cheat
            return false;
        }

        // if we reach here then the attempted move Cheats
        move->is_cheat = true;
        // Columns containing a full stack may not be the target of any move,
        // including a Cheat
        if (fcol->count == 9 && fcol->stack_begin == 0) {
            return false;
        }
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
    int column_if_two_moves(const Board* board, const Move* move) {
        const int from_col = move->from_x;
        const int to_col = move->to_x;
        const Column* fcol = board->cols[from_col];
        if (fcol->cheated) {
            // find the column using the algorithm from can_move
            for (int i = 0; i < 6; i++) {
                if (i == from_col || i == to_col) {
                    continue;
                }
                const Column* col = board->cols[i];
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
    bool solved(const Board* board) {
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
    void apply_move(Board* board, const Move* move) {
        Column* from_col = board->cols[move->from_x];
        Column* to_col = board->cols[move->to_x];
        const uint8 n_elements = from_col->count - move->from_y;
        memcpy(to_col->cards + move->to_y, from_col->cards + move->from_y, n_elements);
        if (move->was_cheat) {
            from_col->cheated = false;
        }
        if (move->is_cheat) {
            to_col->cheated = true;
            to_col->stack_begin = to_col->count;
        }
        from_col->count = move->from_y;
        from_col->stack_begin = stack_begin(from_col);
        to_col->count += n_elements;
    }

    /* Unapply move to board; opposite of apply_move
     */
    void unapply_move(Board* board, const Move* move) {
        Column* from_col = board->cols[move->from_x];
        Column* to_col = board->cols[move->to_x];
        uint8 n_elements = to_col->count - move->to_y;
        memcpy(from_col->cards + move->from_y, to_col->cards + move->to_y, n_elements);
        from_col->stack_begin = from_col->count;
        from_col->count = move->from_y + n_elements;
        to_col->count = move->to_y;
        from_col->cheated = move->was_cheat;
        if (move->is_cheat) {
            to_col->cheated = false;
            to_col->stack_begin = stack_begin(to_col);
        }
    }

    ```
    </details>
  - <details><summary><code>board_info.h</code> (click to expand)</summary>

    ```c
      #pragma once
    #include "types.h"

    bool legal_stack(const Column* col, uint8 position);
    uint8 stack_begin(const Column* col);
    bool can_move(const Board* board, Move* move);
    int column_if_two_moves(const Board* board, const Move* move);
    bool solved(const Board* board);
    void apply_move(Board* board, const Move* move);
    void unapply_move(Board* board, const Move* move);

    ```
    </details>
  - <details><summary><code>compress.c</code> (click to expand)</summary>

    ```c
      #include "compress.h"

    /* Compress a board state to a board state string
     * Return string needs freeing
     */
    string compress(const Board* board) {
        // board state string is always 42 characters: 36 cards + 6 columns
        string out = malloc(42 * sizeof(char));
        uint8 idx = 0;
        for (int col = 0; col < 6; col++) {
            const Column* column = board->cols[col];
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

    bool _compare(const Column* col, string* compressed) {
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
    bool equal(const Board* board, const string compressed) {
        string consumable = malloc(42);
        memcpy(consumable, compressed, 42);
        bool matched[6] = {false, false, false, false, false, false};
        for (int col_compressed = 0; col_compressed < 6; col_compressed++) {
            bool found_match = false;
            for (int col_real = 0; col_real < 6; col_real++) {
                if (matched[col_real]) {
                    continue;
                }
                if (_compare(board->cols[col_real], &consumable)) {
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

    string compress(const Board* board);
    bool equal(const Board* board, const string compressed);

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
    uint32 hash(const Board* board) {
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
    uint32 hash_column(const Column* column) {
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
    uint32 hash(const Board* board);
    uint32 hash_column(const Column* column);

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
    BoardHashTable_LLNode* create_node(const string board_state, int moves) {
        BoardHashTable_LLNode* node = malloc(sizeof(BoardHashTable_LLNode));
        node->board_state = board_state;
        node->unsolvable_in = moves;
        node->next = NULL;
        return node;
    }

    /* Completely deallocate a BoardHashTable linked list
     *
     * Should get a Tail Recursion Optimisation
     */
    void deallocate_list(BoardHashTable_LLNode* node) {
        if (node == NULL) {
            return;
        }
        BoardHashTable_LLNode* next = (BoardHashTable_LLNode*)node->next;
        free((void*)node->board_state);
        free(node);
        deallocate_list(next);
    }

    /* Copy a move
     *
     * Returned move needs freeing
     */
    Move* copy_move(Move* move) {
        Move* ret = malloc(sizeof(Move));
        memcpy(ret, move, sizeof(Move));
        return ret;
    }

    ```
    </details>
  - <details><summary><code>memory.h</code> (click to expand)</summary>

    ```c
      #include "types.h"

    Board* allocate_board();
    void free_board(Board* board);
    BoardHashTable_LLNode* create_node(const string board_state, int moves);
    void deallocate_list(BoardHashTable_LLNode* node);
    Move* copy_move(Move* move);

    ```
    </details>
  - <details><summary><code>parse.c</code> (click to expand)</summary>

    ```c
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

    ```
    </details>
  - <details><summary><code>parse.h</code> (click to expand)</summary>

    ```c
      #pragma once
    #include "board_info.h"
    #include "memory.h"
    #include "types.h"

    Board* parse_input(const string given_state);

    ```
    </details>
  - <details><summary><code>solver.c</code> (click to expand)</summary>

    ```c
      #include "solver.h"

    Move** moves;
    BoardHashTable_LLNode* cache[0x1000000] = {NULL};
    int cache_boundary = 16;
    int continue_millis = 500;
    int max_depth = 1024;
    Time timer = {0, 0};

    /* Clear the cache. Used between no-Cheat and Cheat passes,
     * as well as at the end to prevent memory leaks
     */
    void clear_cache() {
        eprintln("Clearing cache...");
        for (uint32 cache_idx = 0; cache_idx < 0x1000000;) {
            if (cache[cache_idx] != NULL) {
                deallocate_list(cache[cache_idx]);
                cache[cache_idx] = NULL;
            }
            cache_idx++;
        }
        eprintln("Done");
    }

    /* Free moves[idx] if required, and return whether moves[idx] was freed */
    bool free_if_required(int idx) {
        if (moves[idx] != NULL) {
            free(moves[idx]);
            moves[idx] = NULL;
            return true;
        }
        return false;
    }

    /* Free all moves from idx onwards
     */
    void free_moves_from(int idx) {
        // Loop all elements from idx to the end
        for (int i = idx; i < max_depth; i++) {
            if (!free_if_required(i)) {
                // Return if moves[i] is NULL as all other moves will be NULL
                return;
            }
        }
    }

    /* Search recursively for the next move; backtracking.
     * Return -1 if no valid moves found, non-negative to
     * indicate number of moves.
     *
     * Cheating should now be implemented
     */
    int step(Board* board, int depth, int max_moves, bool allow_cheat) {
        if (depth >= max_moves) {
            return -1;
        }
        int remaining_moves = max_moves - depth;
        uint32 board_hash = hash(board) & 0xFFFFFF;
        BoardHashTable_LLNode* node = cache[board_hash];
        BoardHashTable_LLNode* last_node = NULL;
        while (node != NULL && !equal(board, (const string)node->board_state)) {
            node = (BoardHashTable_LLNode*)(last_node = node)->next;
        }
        if (node && node->unsolvable_in >= remaining_moves) {
            return -1;
        }
        // Add the node to the cache unconditionally
        // This prevents the solver from entering loops
        bool free_node = false;
        if (node) {
            node->unsolvable_in = remaining_moves;
        } else {
            free_node = true;
            string compressed = compress(board);
            if (last_node) {
                last_node->next = create_node(compressed, remaining_moves);
                node = (BoardHashTable_LLNode*)last_node->next;
            } else {
                node = cache[board_hash] = create_node(compressed, remaining_moves);
            }
        }
        Move* move = malloc(sizeof(Move));
        Move* cheat_step_one = malloc(sizeof(Move));
        Move* cheat_step_two = malloc(sizeof(Move));
        int two_move_cheat_col;
        bool found_solution = false;
        // we want to start with non-Cheats only as these are more likely to work
        for (move->from_x = 0; move->from_x < 6; move->from_x++) {
            for (move->to_x = 0; move->to_x < 6; move->to_x++) {
                if (move->from_x == move->to_x) {
                    continue;
                }
                if (board->cols[move->from_x]->count == 0) {
                    continue;
                }
                // If we've run out of time, return; otherwise, keep looking for a shorter
                // solution
                if (expired(timer)) {
                    goto finalise;
                }
                move->from_y = board->cols[move->from_x]->stack_begin;
                move->to_y = board->cols[move->to_x]->count;
                move->was_cheat = board->cols[move->from_x]->cheated;
                if (!can_move(board, move)) {
                    continue;
                }
                if (move->is_cheat) {
                    continue;
                }
                apply_move(board, move);
                if (solved(board)) {
                    // Solving the board this move is the best I can do; start a timer and
                    // return
                    found_solution = true;
                    free_moves_from(depth);
                    moves[depth] = copy_move(move);
                    // If the timer is not already running, start it
                    start(timer, continue_millis);
                    max_moves = depth + 1;
                    unapply_move(board, move);
                    goto finalise;
                }
                int found = step(board, depth + 1, max_moves, allow_cheat);
                if (found != -1) {
                    // Found a solution in fewer moves than before
                    found_solution = true;
                    free_if_required(depth);
                    moves[depth] = copy_move(move);
                    max_moves = found;
                    // If the timer is not already running, start it
                    start(timer, continue_millis);
                }
                unapply_move(board, move);
            }
        }
        if (allow_cheat) {
            // we couldn't solve it without Cheating here; try Cheating now
            for (move->from_x = 0; move->from_x < 6; move->from_x++) {
                for (move->to_x = 0; move->to_x < 6; move->to_x++) {
                    if (move->from_x == move->to_x) {
                        continue;
                    }
                    if (board->cols[move->from_x]->count == 0) {
                        continue;
                    }
                    // If we've run out of time, return; otherwise, keep looking for a
                    // shorter solution
                    if (expired(timer)) {
                        goto finalise;
                    }
                    move->from_y = board->cols[move->from_x]->stack_begin;
                    move->to_y = board->cols[move->to_x]->count;
                    move->was_cheat = board->cols[move->from_x]->cheated;
                    if (!can_move(board, move)) {
                        continue;
                    }
                    if (!move->is_cheat) {
                        continue;
                    }
                    two_move_cheat_col = column_if_two_moves(board, move);
                    const int two_move_col = two_move_cheat_col;
                    const bool is_two_move = two_move_col != -1;
                    if (is_two_move) {
                        // Set up the two steps using the data from move and two_move_col
                        cheat_step_one->from_x = two_move_col;
                        cheat_step_one->from_y = board->cols[two_move_col]->count - 1;
                        cheat_step_one->to_x = move->to_x;
                        cheat_step_one->to_y = move->to_y;
                        cheat_step_one->is_cheat = true;
                        cheat_step_two->from_x = move->from_x;
                        cheat_step_two->from_y = move->from_y;
                        cheat_step_two->to_x = two_move_col;
                        cheat_step_two->to_y = board->cols[two_move_col]->count - 1;
                        cheat_step_two->is_cheat = false;

                        // Remember that this is a two-step Cheat in order to
                        // finalise correctly
                        depth++;
                    }
                    apply_move(board, move);
                    if (solved(board)) {
                        // Solving the board this move is the best I can do; start a timer
                        // and return
                        found_solution = true;
                        free_moves_from(depth);
                        if (is_two_move) {
                            free_if_required(depth - 1);
                            moves[depth - 1] = copy_move(cheat_step_one);
                            moves[depth] = copy_move(cheat_step_two);
                        } else {
                            moves[depth] = copy_move(move);
                        }
                        // If the timer is not already running, start it
                        start(timer, continue_millis);
                        max_moves = depth + 1;
                        unapply_move(board, move);
                        goto finalise;
                    }
                    int found = step(board, depth + 1, max_moves, allow_cheat);
                    if (found != -1) {
                        // Found a solution in fewer moves than before
                        found_solution = true;
                        free_if_required(depth);
                        if (is_two_move) {
                            free_if_required(depth - 1);
                            moves[depth - 1] = copy_move(cheat_step_one);
                            moves[depth] = copy_move(cheat_step_two);
                        } else {
                            moves[depth] = copy_move(move);
                            // Found a solution better than any found double-Cheat
                            // so remember that this ISN'T a double-Cheat
                        }
                        max_moves = found;
                        // If the timer is not already running, start it
                        start(timer, continue_millis);
                    }
                    unapply_move(board, move);
                    if (is_two_move) {
                        // Decrement depth to return to the correct place
                        depth--;
                    }
                }
            }
        }
    finalise:
        // Cut the node out if I had to allocate it and we're not within the minimum cache
        // boundary, or if I've proven that it's possible to solve the board
        if (found_solution
            || (free_node && cache_boundary != -1 && remaining_moves > cache_boundary)) {
            if (last_node) {
                last_node->next = node->next;
            } else {
                cache[board_hash] = (BoardHashTable_LLNode*)node->next;
            }
            free((void*)node->board_state);
            free(node);
        }
        // Free the loop variables to avoid a memory leak
        free(move);
        free(cheat_step_one);
        free(cheat_step_two);
        // If I found a solution, return the number of moves it took
        if (found_solution) {
            return max_moves;
        }
        return -1;
    }

    int main(int argc, string argv[]) {
        extern string optarg;
        extern int optind, optopt;
        bool solver_allow_cheat = false;

        int opt;
        int exit_code = EXIT_FAILURE;
        int min_max_depth = 64;
        while ((opt = getopt(argc, argv, ":chn:t:m:d:")) != -1) {
            switch (opt) {
                case 'c':
                    solver_allow_cheat = true;
                    break;
                case 'n':
                    cache_boundary = atoi(optarg);
                    break;
                case 'd':
                    min_max_depth = atoi(optarg);
                    break;
                case 't':
                    continue_millis = atoi(optarg);
                    break;
                case 'm':
                    max_depth = atoi(optarg);
                    break;
                case '?':
                    eprintfln("%s: -%c: invalid option", argv[0], optopt);
                    goto print_help;
                case ':':
                    eprintfln("%s: -%c: option requires an argument", argv[0], optopt);
                    goto print_help;
                case 'h':
                    exit_code = EXIT_SUCCESS;
                    goto print_help;  // this line suppresses -Werror=implicit-fallthrough
                default:
                print_help:
                    eprintfln(
                        "Usage: %s [-ch] [-n <moves>=16] [-t <milliseconds>=500] [-m <max "
                        "moves>=1024] <STATE>",
                        argv[0]);
                    if (exit_code == EXIT_SUCCESS) {
                        eprintln("  -c                  Allow Cheating.");
                        eprintln(
                            "                      The solver will only attempt to Cheat "
                            "if a solution cannot be found without Cheating.");
                        eprintln("  -h                  Print this message and exit.");
                        eprintln(
                            "  -n <cache boundary> The maximum number of moves remaining "
                            "for the board state to be cached.");
                        eprintln("                      Defaults to 16.");
                        eprintln("                      If set to -1, all board states are "
                                 "cached. (Not recommended).");
                        eprintln("  -t <milliseconds>   How long to continue searching for "
                                 "a more optimal solution after one is found.");
                        eprintln("                      Defaults to 500 (0.5 seconds).");
                        eprintln(
                            "                      If set to -1, the solver will search "
                            "all possibilities to find the most optimal solution.");
                        eprintln("  -d <maximum moves>  The minimum maximum number of "
                                 "moves to allow.");
                        eprintln("                      Defaults to 64.");
                        eprintln(
                            "                      Setting this to very low or very high "
                            "values will increase run time and memory consumption.");
                        eprintln(
                            "  -m <maximum moves>  The maximum number of moves to allow.");
                        eprintln("                      Defaults to 1024.");
                        eprintln("                      Very large values will cause "
                                 "allocation errors.");
                        eprintln("                      It is recommended to make this a "
                                 "multiple of 4 as the solver will only attempt to solve "
                                 "with maximum depths of multiples of 4.");
                    }
                    exit(exit_code);
            }
        }
        if (!argv[optind]) {
            println("Need starting board state");
            exit(EXIT_FAILURE);
        }
        Board* board = parse_input(argv[optind]);
        // invalid boards will cause an exit in parse_input

        int start_max_depth = min(min_max_depth, max_depth);
        moves = calloc(max_depth, sizeof(Move*));

        for (int step_max_depth = start_max_depth; step_max_depth <= max_depth;
             step_max_depth += 4) {
            int n_moves = step(board, 0, step_max_depth, false);
            if (n_moves == -1) {
                eprintf("No solution found in %d moves\r", step_max_depth);
            } else {
                for (int i = 0; i < n_moves; i++) {
                    printfln(
                        "%d %d %c> %d %d",
                        moves[i]->from_x,
                        moves[i]->from_y,
                        moves[i]->is_cheat ? '!' : '-',
                        moves[i]->to_x,
                        moves[i]->to_y);
                    free(moves[i]);
                }
                free_board(board);
                clear_cache();
                return 0;
            }
        }
        if (solver_allow_cheat) {
            clear_cache();
            for (int step_max_depth = start_max_depth; step_max_depth <= max_depth;
                 step_max_depth += 4) {
                int n_moves = step(board, 0, step_max_depth, true);
                if (n_moves == -1) {
                    eprintf("No solution found in %d moves (Cheating)\r", step_max_depth);
                } else {
                    for (int i = 0; i < n_moves; i++) {
                        printfln(
                            "%d %d %c> %d %d",
                            moves[i]->from_x,
                            moves[i]->from_y,
                            moves[i]->is_cheat ? '!' : '-',
                            moves[i]->to_x,
                            moves[i]->to_y);
                        free(moves[i]);
                    }
                    free_board(board);
                    clear_cache();
                    return 0;
                }
            }
        }
        free_board(board);
        clear_cache();
        return 0;
    }

    ```
    </details>
  - <details><summary><code>solver.h</code> (click to expand)</summary>

    ```c
      #pragma once
    #include "board_info.h"
    #include "compress.h"
    #include "hash.h"
    #include "memory.h"
    #include "parse.h"
    #include "timer.h"
    #include "types.h"
    #ifdef DEBUG
        #include "debug.h"
    #endif

    #include <unistd.h>

    int step(Board* board, int depth, int max_moves, bool allow_cheat);

    ```
    </details>
  - <details><summary><code>timer.c</code> (click to expand)</summary>

    ```c
      #include "timer.h"

    /* Create a timer with extra_milliseconds on the clock
     */
    Time make_timer(int extra_milliseconds) {
        // If extra_milliseconds == -1 then the user has disabled the timer (so don't start
        // one)
        if (extra_milliseconds == -1) {
            return (Time) {0, 0};
        }
        Time now;
        gettimeofday(&now, NULL);
        now.tv_usec += 1000 * extra_milliseconds;
        now.tv_sec += now.tv_usec / 1000000;
        now.tv_usec %= 1000000;
        return now;
    }

    /* Check if timer has expired
     */
    bool expired(Time timer) {
        // timer has not been started; it is not expired
        if (timer.tv_sec == 0) {
            return false;
        }
        Time now;
        gettimeofday(&now, NULL);
        return now.tv_sec > timer.tv_sec
               || (now.tv_sec == timer.tv_sec && now.tv_usec > timer.tv_usec);
    }

    ```
    </details>
  - <details><summary><code>timer.h</code> (click to expand)</summary>

    ```c
      #pragma once
    #include "common.h"

    #include <sys/time.h>

    #define start(timer, time)        \
        if (!timer.tv_sec) {          \
            timer = make_timer(time); \
        }

    typedef struct timeval Time;

    Time make_timer(int extra_milliseconds);
    bool expired(Time timer);

    ```
    </details>
  - <details><summary><code>types.h</code> (click to expand)</summary>

    ```c
      #pragma once
    #include "common.h"

    typedef uint8 Card;

    typedef struct _Column {
        // largest possible stack:
        // XXXXXTKDV09876C (15)
        Card cards[15];
        uint8 count;
        uint8 stack_begin;
        bool cheated;
    } Column;

    typedef struct _Board {
        Column* cols[6];  // Board.cols[x][y]
    } Board;

    typedef struct _Move {
        uint8 from_x;
        uint8 from_y;
        uint8 to_x;
        uint8 to_y;
        bool is_cheat;
        bool was_cheat;
    } Move;

    typedef struct _BoardHashTable_LLNode {
        const char* board_state;
        int unsolvable_in;
        const struct _BoardHashTable_LLNode* next;
    } BoardHashTable_LLNode;

    ```
    </details>
