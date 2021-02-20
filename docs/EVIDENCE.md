# Evidence of Project Development

## Principles

- Modular development
  - The solver is broken up into small modules which each handle their own tasks; the program contains 8 individual modules (`board_info`, `common` `compress` `hash` `parse` `memory` `timer`) which each perform their own tasks and are used by the main program (Note that `common` is a general-purpose module I lifted straight from my personal C project folder, and as such many of its functions go unused in this project).
  - For example, `board_info` handles the board; from [its header file][1]:

    ```c
    bool legal_stack(const Column* col, uint8 position);
    uint8 stack_begin(const Column* col);
    bool can_move(const Board* board, Move* move);
    int column_if_two_moves(const Board* board, const Move* move);
    bool solved(const Board* board);
    void apply_move(Board* board, const Move* move);
    void unapply_move(Board* board, const Move* move);
    ```

    That is, it can:
    - Determine if stacking from a point is legal
    - Determine where the beginning of a stack is
    - Determine where the intermediary column of a two-step Cheat is
    - Determine if a `Board` is solved
    - Apply a `Move`; that is, perform the action described by `move` on the `board`
    - Unapply a `Move`; that is, the reverse of applying it (necessary for the algorithm to backtrack)
  - For an alternative example, `memory` handles all the memory related tasks (allocation and freeing of complex structures); from [its header file][2]:

    ```c
    Board* allocate_board();
    void free_board(Board* board);
    BoardHashTable_LLNode* create_node(const string board_state, int moves);
    void deallocate_list(BoardHashTable_LLNode* node);
    Move* copy_move(Move* move);
    ```

    Its responsibilities are:
    - Creation and destruction of `Board`s (handled with pointers to allow mutability, and is the reason these functions are necessary)
    - Creation of hash table nodes (also handled with pointers, as this is the only way to link them together)
    - Destruction of hash table linked lists (performed recursively, but likely to be tail-call optimised by the compiler)
    - Copying `Move`s (used to place moves on the stack while allowing mutation, so the solver may continue attempting to optimise its solution)
- Annotation of the code
  - All main functions in the solver's code are annotated, for example [the `can_move` function][3]:

    ```c
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
    ```

    This function has a documentation string, and is littered with comments describing what it's doing and why
- Meaningful variable names
  - The vast majority of the solver's code uses meaningful variable names (with a few simpler functions using shorter, but still understandable, names such as `fcol` for the from column, or `i` for loop counters)
  - A good example is [`parse_input` from `parse.c`][4]:

    ```c
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

    Each variable is named in a way that makes it very easy to infer the variable's purpose, such as `this_column`, which is the column being parsed, or `num_cards_found`, which is the number of cards that have been found; etc.
- Validation
  - The above code sample is also an example of validation, as there are numerous points where if invalid data is found, the user is informed and the program terminates (with a failure exit code)
- Use of appropriate data structures for storing data
  - The solver uses a selection of data structures, some specialised (`Board`, `Column`, and `Move`), and some more general (arrays of various types, linked lists for the hash table)
  - These can be seen within [`types.h`][5]:

    ```c
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

  - Every type used by the solver is defined in this file, with a brief explanation of their design where it isn't obvious - for instance, the hash table nodes contain a compressed board state used to ensure there are no hash collisions, an integer representing the minimum number of moves a step may have remaining in order for it to even attempt solving the state, and a pointer to the next node
- Error handling
  - Using `-Wall -Wextra -Werror -pedantic-errors` at compile time ensures that my code avoids the vast majority of possible errors; in addition, since the user's input is validated (and rejected if it's bad), there are no places a bug is likely to sneak in (even a segmentation fault due to an incredibly high `-m` is impossible, as calloc is an *optimistic* memory allocator, and it has been tested on both supported operating systems with `-m 2147483647`, the largest possible value)

[1]: https://github.com/Starwort/NEA/blob/master/solver_c/board_info.h "board_info.h"
[2]: https://github.com/Starwort/NEA/blob/master/solver_c/memory.h "memory.h"
[3]: https://github.com/Starwort/NEA/blob/master/solver_c/board_info.c#L33-L104 "board_info.c, can_move, lines 33-104"
[4]: https://github.com/Starwort/NEA/blob/master/solver_c/parse.c#L3-L99 "parse.c, parse_input, lines 3-99"
[5]: https://github.com/Starwort/NEA/blob/master/solver_c/types.h "types.h"
