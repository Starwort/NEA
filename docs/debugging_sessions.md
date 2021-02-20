# Debugging sessions

## Session: 2021-01-20

- Compiled with:
  - `-O3 -g -lefence`
- Relevant GDB session log:

  - ```
    (gdb) print compressed
    $11 = (string) 0x55555d55ce60 "8TK7DK9\207\067\061DV6T\200\310\060Q000000003561", '0' <repeats 19 times>, "q000000Q0000000879DV6T000000000....000000000\220\364!V076VD6."
    (gdb) print board
    $12 = (Board *) 0x55555d55c2a0
    (gdb) print *board
    $13 = {cols = {0x55555d55c2e0, 0x55555d55c300, 0x55555d55c320, 0x55555d55c340, 0x55555d55c360, 0x55555d55c380}, depth = 0}
    (gdb) print *board->cols[0]
    $14 = {cards = "\b\016\r\a\f\r\tW\a\001\f\v\006\016", count = 80 'P', stack_begin = 152 '\230', cheated = false}
    ```

- Conclusion:
  - Something bad is going wrong
  - Somewhere, something is being corrupted
  - The `cards` buffer is being written to with garbage data far into out of bounds, resulting in the obvious visible garbage data (`count` = 80, `stack_begin` = 152)

## Session: 2021-01-21

- Compiled with:
  - `DEBUG=True`
- Relevant GDB session log:

  - ```
    (gdb) set args -c 997VT8.076VD6.879DV6.T00KD8.09T6K8.VTK7DK.
    (gdb) break solver.c:137
    Breakpoint 1 at 0x13c3: file solver_c/solver.c, line 138.
    (gdb) run
    Starting program: /home/starwort/Documents/NEA/dist/solver -c 997VT8.076VD6.879DV6.T00KD8.09T6K8.VTK7DK.
    No solution found in 64 moves
    No solution found in 128 moves
    No solution found in 256 moves
    No solution found in 512 moves
    No solution found in 1024 moves
    Clearing cache...

    Breakpoint 1, main (argc=<optimised out>, argv=<optimised out>)
        at solver_c/solver.c:138
    138             for (uint32 cache_idx = 0; cache_idx < 0x1000000;) {
    (gdb) break fprintf
    Breakpoint 2 at 0x5555555552a0: fprintf. (6 locations)
    (gdb) continue
    Continuing.

    Breakpoint 2, fprintf (__fmt=0x5555555571e4 "Done\n", 
        __stream=0x7ffff7fa05c0 <_IO_2_1_stderr_>)
        at /usr/include/x86_64-linux-gnu/bits/stdio2.h:100
    100       return __fprintf_chk (__stream, __USE_FORTIFY_LEVEL - 1, __fmt,
    (gdb) up
    #1  main (argc=<optimised out>, argv=<optimised out>)
        at solver_c/solver.c:145
    145             eprintln("Done");
    (gdb) up
    Initial frame selected; you cannot go up.
    (gdb) print *board
    value has been optimised out
    ```

- Conclusion:
  - Something bad is going wrong
  - For some reason, `board` is optimised out even before it is done being used

- Compiled with:
  - `DEBUG=True`
- Relevant GDB session log:

  - ```
    (gdb) print *board
    $1 = {cols = {0x55555d55c2e0, 0x55555d55c300, 0x55555d55c320, 
        0x55555d55c340, 0x55555d55c360, 0x55555d55c380}, depth = 0}
    (gdb) print *board->cols[0]
    $2 = {cards = "\375\063\320\t\t\a\v\016\b\000\000\000\000", count = 9 '\t', 
      stack_begin = 8 '\b', cheated = false}
    ```

- Conclusion:
  - Somehow, the garbage data `253, 51, 208` is being written to the beginning of the array of cards
