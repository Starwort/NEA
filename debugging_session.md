# Debugging session: 2021-01-20

- Compiled with:
  - `-O3 -g -lefence`
- Relevant GDB session log:

  - ```gdb
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
