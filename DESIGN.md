# Design of the Solitaire Solver

## Decomposition

The problem has been broken down as shown below:

1. Solver (written in C)
   1. Board state and parameter parsing
      1. Board state parsing is handled by my code
         - I decided to encode the board state in a simple-to-parse way in order to ease the writing of the solver
      2. Argument parsing is handled by `getopt`
         - This is a standard C module which is used to handle options
   2. Backtracking solution search
      - The solution search is recursive, using a backtracking algorithm:
        - to solve a given board:
          - see if the board is solved
            - if it is, return no moves
          - for each available move:
            - make the move
            - try to solve the new board
              - if it was solved, return this move and those moves
              - otherwise, undo the move
          - there is no sequence of moves which can solve this board
      - The real program differs slightly from this in order (the 'board is solved' check occurs after the move rather than at the beginning of the recursive step) and implementation (the sequence of moves is stored in a global array rather than returned dynamically from the functions, as C cannot do this easily)
      - Additionally, the solution search is repeated up to twice:
        - Once without Cheating (always)
          - If the board is possible to solve without Cheating, it is much faster to find this solution
        - Once with Cheating (only if Cheating is enabled, only if the first pass found no solution)
          - This can be *much* slower than searching without Cheating (finding a solution without Cheating can take under a tenth of a second, but with Cheating enabled can take several minutes)
   3. Solution output
      - The solution is printed to STDOUT
      - Each move in the solution is printed on its own line
        - Moves which do not Cheat are printed as `X Y -> TO_X TO_Y`, where `X`, `Y`, `TO_X`, and `TO_Y` represent the columns and rows of the card's source and destination
        - Moves which Cheat are printed as `X Y !> TO_X TO_Y`, where `X`, `Y`, `TO_X`, and `TO_Y` represent the columns and rows of the card's source and destination. Note that the only difference is in the arrow, and therefore users (or applications) who do not care if the solution Cheats or not can ignore it
2. Automation (written in Python)
   1. Vision
      1. Screenshot capture
         - Screenshot capture is delegated to the library `PyAutoGUI`. Where `PyGetWindow` (a requirement of `PyAutoGUI`) is available, the screenshot capture should* be more sophisticated

           *Assuming my function works correctly - I have not been able to test this as my primary development system is not supported by `PyGetWindow`
      2. Screenshot parsing
         - The game has a 'Pixel-Perfect' scaling mode, and therefore the vision is simpler than if anti-aliasing needed to be accounted for
         - The game screenshot can be scaled and cropped to the base resolution (960x540), and the positions of the cards can be calculated with a set of constants:
           - The column width, in pixels (82)
           - The row height, in pixels (16)
           - The position of the top left card, as a set of pixel co-ordinates, where the origin is at the top left of the screen ((244, 149))
         - Each card in the 6x6 grid (as the vision solver always starts a new game before recognising the image) can be identified using a 8x9 picture of the card's rank (these can be found in the `samples` directory), which are loaded in as constants and compared to the card (as found by cropping the full screenshot to the position of the card) using the `ImageChops` module of `PIL` (the Python Imaging Library, or rather its fork `Pillow`)
         - The board identification function then returns a 6-tuple of 6-tuples of 1-character strings
   2. Solver interface
      - The solver interface takes the Python representation of the board, as well as the required flags, translates them for the solver, and calls the solver.
        - There is also a wrapper function which will use the vision interface to determine the board on-screen
      - The function will determine the appropriate solver executable to use.
      - If running on Linux or MacOS, it will attempt to build the solver using `make` (which will not attempt to actually build the solver if it has done already).
      - It will then construct an argument list, using the determined solver executable path, the provided flags, and the provided board state, and uses `subprocess` to run the solver. `subprocess` is also used to enforce a timeout on the solver, so that it does not take too long (the default timeout is 10 seconds, more than enough for most boards).
      - Using the output of the solver, the function can parse the output into a list of moves which it will return, and optionally print some stats about the found solution.
   3. Input emulation
      - The input emulation interface contains only a couple of functions:
        - `click`, which clicks (and is used as an alternative to `pyautogui.click` as the clicks from this function fail to register)
        - `run_solution`, which takes a list of moves and executes the given solution. Optionally, it can also click the 'new game' button.

## Analysis with Data-Oriented design

The following data structures (which can be found within `types.h`) were needed for the C code:

- `Card`, which is an unsigned 8-bit number representing the value of a card.
- `Column`, which is a structure representing a single column, containing:
  - A fixed-size array of 15 `Card`s (large enough for the largest possible stack of cards: `XXXXXTKDV09876C`, where `X` is any card and `C` is any Cheated card)
  - An unsigned 8-bit integer, which stores the number of cards in this `Column`
  - An unsigned 8-bit integer, which stores the position of the beginning of the stack of `Card`s which may be moved together within this `Column`
  - A boolean containing whether or not this `Column` contains a Cheated `Card`
- `Board`, which is a structure representing the entire board, containing 6 `Column` pointers
- `Move`, which is a structure representing a single move, containing:
  - An unsigned 8-bit integer representing the column the card is moving from (as an index)
  - An unsigned 8-bit integer representing the row the card is moving from (as an index)
  - An unsigned 8-bit integer representing the column the card is moving to (as an index)
  - An unsigned 8-bit integer representing the row the card is moving to (as an index)
  - A boolean containing whether or not the move being made is a Cheat
  - A boolean containing whether or not the `Card` being moved was Cheated previously
- `BoardHashTable_LLNode`, which is a linked-list node for use in the hash table (an optimisation enabling the solver to prematurely discard states, based on whether it has encoutered them before or not)
