# Test log

This will log the tests, their output, and whether the test was successful

## Test #1: Board State Parsing

[Commit of test (used to view the test at the time)](https://github.com/Starwort/NEA/commit/ee750d115fd0a794eb092f42938e4e3377302c9c)

```
starwort@hedwig ~/Documents/NEA master$ dist/test 668790.7V6D87.T87D9K.V0T980.DV6KTK.0DVKT9. TKDV09.876TKD.V09876.TKDV09.876TKD.V09876. TKDV09876.TKDV09876.TKDV09876.TKDV09876... 'TKDV09878!TKDV09878.TKDV09766!TKDV09766...'
668790.7V6D87.T87D9K.V0T980.DV6KTK.0DVKT9.:
 06  07  14  11  12  10 
 06  11  08  10  11  12 
 08  06  07  14  06  11 
 07  12  12  09  13  13 
 09 >08< 09  08 >14< 14 
>10< 07 >13<>10< 13 >09<
^00^^00^^00^^00^^00^^00^
 00  00  00  00  00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
TKDV09.876TKD.V09876.TKDV09.876TKD.V09876.:
>14< 08 >11<>14< 08 >11<
 13  07  10  13  07  10 
 12  06  09  12  06  09 
 11 >14< 08  11 >14< 08 
 10  13  07  10  13  07 
 09  12  06  09  12  06 
^00^^00^^00^^00^^00^^00^
 00  00  00  00  00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
TKDV09876.TKDV09876.TKDV09876.TKDV09876...:
>14<>14<>14<>14<^00^^00^
 13  13  13  13  00  00 
 12  12  12  12  00  00 
 11  11  11  11  00  00 
 10  10  10  10  00  00 
 09  09  09  09  00  00 
 08  08  08  08  00  00 
 07  07  07  07  00  00 
 06  06  06  06  00  00 
^00^^00^^00^^00^ 00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
TKDV09878!TKDV09878.TKDV09766!TKDV09766...:
>14<>14<>14<>14<^00^^00^
 13  13  13  13  00  00 
 12  12  12  12  00  00 
 11  11  11  11  00  00 
 10  10  10  10  00  00 
 09  09  09  09  00  00 
 08  08  07  07  00  00 
 07  07  06  06  00  00 
!08! 08 !06! 06  00  00 
^00^^00^^00^^00^ 00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
 00  00  00  00  00  00 
```

Test result: 3 success, 1 failure

Post-mortem: Bit field limited to 3 bits due to false assumption that valid inputs may never have stacks beginning on a y greater than 7; 'legitimate' legal states may not, but 'cheated' legal states may (columns 1 and 3 of example 4). Note that columns 2 and 4 of example 4 are illegal states that will still be handled gracefully by the parser, after the fix to `types.h`.

## Test #2: `stack_begin()` profiling/benchmarking

[Commit of test (used to view the test at the time)](https://github.com/Starwort/NEA/commit/a23ce7b2f4f4f0bfc521bbfe2dc150b6c92d8b72)

```
starwort@hedwig ~/Documents/NEA master$ dist/test TKDV09876.TKDV09876.TKDV09876.TKDV09876...
After 10,000,000 iterations:
Elapsed: 0s 338,716µs
Avg. time per iteration: 33.871601ns
starwort@hedwig ~/Documents/NEA master$ dist/test TKDV09876.TKDV09876.TKDV09876.TKDV09876... 10000000000
Using 10,000,000,000 iterations (instead of 10,000,000 iterations)
After 10,000,000,000 iterations:
Elapsed: 292s 945,474µs
Avg. time per iteration: 29.294546ns
```

Test result: Successful; 10,000,000 calls of the function (in the worst case) can run in under half a second; each worst-case call takes approximately 29.3ns on average.

## Test #3: Board compression and comparison

[Commit of test (used to view the test at the time)](https://github.com/Starwort/NEA/commit/8a86fec1f35856f467f279d8ca910350584e66fd)

```
starwort@hedwig ~/Documents/NEA master$ dist/test 668790.7V6D87.T87D9K.V0T980.DV6KTK.0DVKT9. TKDV09.876TKD.V09876.TKDV09.876TKD.V09876. TKDV09876.TKDV09876.TKDV09876.TKDV09876... 'TKDV09878!TKDV09878.TKDV09766!TKDV09766...'
Original:      668790.7V6D87.T87D9K.V0T980.DV6KTK.0DVKT9.
Compressed:    668790.7V6D87.T87D9K.V0T980.DV6KTK.0DVKT9.
Identical:     true
Compare equal: true
Original:      TKDV09.876TKD.V09876.TKDV09.876TKD.V09876.
Compressed:    TKDV09.876TKD.V09876.TKDV09.876TKD.V09876.
Identical:     true
Compare equal: true
Original:      TKDV09876.TKDV09876.TKDV09876.TKDV09876...
Compressed:    TKDV09876.TKDV09876.TKDV09876.TKDV09876...
Identical:     true
Compare equal: true
Original:      TKDV09878!TKDV09878.TKDV09766!TKDV09766...
Compressed:    TKDV09878!TKDV09878.TKDV09766!TKDV09766...
Identical:     true
Compare equal: true
```

Test result: Successful; Compression is the inverse of parsing, and comparison of an inflated (`Board*`) state and a compressed (`string`/`char*`) state works correctly.

## Test #4: Board state permutation invariance (comparison)

[Commit of test (used to view the test at the time)](https://github.com/Starwort/NEA/commit/c7bc22b7a4cac4e2709f71ed49e2df7cbbedc77b)

Output: [![asciicast](https://asciinema.org/a/2JbOBIMBh1DxGX0zAnPkRYNl3.svg)](https://asciinema.org/a/2JbOBIMBh1DxGX0zAnPkRYNl3)

```
starwort@hedwig ~/Documents/NEA master$ bash -c '(read a; while read b; do dist/test \"$a\" \"$b\"; done) < samples/permutations.txt' | grep -B 2 false
```

(Author's note: I use [xonsh](https://xon.sh/) so the bash snippet needs to be emulated in bash)

Test result: Successful; All permutations of the board state are accepted as equivalent in comparison

## Test #5: Board state permutation invariance (hashing)

[Commit of test (used to view the test at the time)](https://github.com/Starwort/NEA/commit/4b3894199036135bc13e0ffa85358459d9ae34c8)

```
starwort@hedwig ~/Documents/NEA master$ bash -c '(while read b; do dist/test \"$b\"; done) < samples/permutations.txt' | python -c "import sys; print(set(sys.stdin.read().splitlines()))"
{'57555068'}
```

Test result: Successful; All permutations of the board state are accepted as equivalent in hashing (single result over all permutations)

## Test #6: Hash collision likelihood

[Commit of test (used to view the test at the time)](https://github.com/Starwort/NEA/commit/4b3894199036135bc13e0ffa85358459d9ae34c8)

```
starwort@hedwig ~/Documents/NEA master$ xargs -a samples/toy.txt dist/test | python -c "import sys; print(_:=set(sys.stdin.read().splitlines()), len(_))"; cat samples/toy.txt | wc -l
{'228303792', '571845447', '1756605383', '1030069549', '328575377', '1811331921', '1590868318', '22301669', '1238129521', '1958393456', '56924340', '1245889', '919853135', '1563027505', '1507860214', '756225619'} 16
15
```

Test result: Successful; 16 different hashes are generated; one per board state in the sample file (note that `wc` has an off-by-one error due to `samples/toy.txt` not containing a trailing newline)

## Test #7: Hash collision (Cheating)

[Commit of test (used to view the test at the time)](https://github.com/Starwort/NEA/commit/4b3894199036135bc13e0ffa85358459d9ae34c8)

```
starwort@hedwig ~/Documents/NEA master$ dist/test 668790.7V6D87.T87D9K.V0T980.DV6KTK.0DVKT9.; dist/test "668790.7V6D87.T87D9K.V0T980.DV6KTK.0DVKT9!"
228303792
748997643
```

Test result: Successful; the board states differ only by the cheat state of column 5, and the hash is different

## Test #8: Optimal solution search

[Commit of test (used to view the test at the time)](https://github.com/Starwort/NEA/commit/1472de4cb7bc1216b1e920f19927db1876bba531)

- Test:

  - ```python
    for line in $(cat samples/toy.txt).splitlines():
        first = $(dist/solver @(line) -t 0)
        best = $(dist/solver @(line) -t 10000)
        if first != best:
            print(f'Found a more optimal solution for board {line}')
            break
    ```

- Explanation of how the test works:
  - For every example within `samples/toy.txt`,
    - Run the solver with an optimisation time of 0 milliseconds (do not attempt to optimise)
      - This finds the solver's first valid solution
    - Run the solver again with an optimisation time of 10,000 milliseconds (10 seconds)
      - This finds an optimal solution without taking forever
        - Using `-1` in order to search all possibilities is a bad idea
    - If the optimal solution differs from the initial solution, then say so and exit
- Output of the test:

  - ```
    Found a more optimal solution for board VTK0KV.89K97V.697070.6TT880.D9T6K6.D7V8DD.
    ```

- Conclusion:
  - The solver successfully searches for (and finds) an additional solution, given enough time

## Test #9: `parse_input` robustness

[Commit of test (used to view the test at the time)](https://github.com/Starwort/NEA/commit/72d426b83270a9aaa66a40963c8af0695ae8ae2a/)

```
starwort@hedwig ~/Documents/NEA master$ dist/test "NotEnoughCharacters"
Unexpected end of string; Expected 42 characters but only got 19
starwort@hedwig ~/Documents/NEA master$ _.rtn
1
starwort@hedwig ~/Documents/NEA master$ dist/test "MoreCharsThanNecessarySoTheUnnecessaryCharsGetIgnored!"
Bad input string: Expected 6 columns, got 0
starwort@hedwig ~/Documents/NEA master$ _.rtn
1
starwort@hedwig ~/Documents/NEA master$ dist/test "Enough Characters But No Column Separators"
Bad input string: Expected 6 columns, got 0
starwort@hedwig ~/Documents/NEA master$ _.rtn
1
starwort@hedwig ~/Documents/NEA master$ dist/test "Enough.CharsAndSeps.But.All.Character.Bad!"
Expected a digit in the range 6-9, '0', 'T', 'K', 'D', V, '!', or '.'; got 'E'
starwort@hedwig ~/Documents/NEA master$ _.rtn
1
starwort@hedwig ~/Documents/NEA master$ dist/test "VTK0KV.89K97V.697070.6TT880.D9T6K6.D7V8D.D" # Column sep in wrong place (extra card in a bad 7th col, card missing from 6th col)
starwort@hedwig ~/Documents/NEA master$ _.rtn
0
```

Test result: Mostly successful; 3 success, 1 fail: All malformed inputs, except those that are formatted validly but with an invalid layout of cards, get rejected (note that inputs with extra characters are effectively truncated at the 42nd character [note the number of columns being 0 in the second example], this is intended behaviour)

Test action: Validate the number of cards parsed after parsing the board state; there must be exactly 36 cards within the board layout

Post-resolution: Test case now passes

```
starwort@hedwig ~/Documents/NEA master$ dist/test "VTK0KV.89K97V.697070.6TT880.D9T6K6.D7V8D.D"
Bad input string: Expected 36 cards, got 35
starwort@hedwig ~/Documents/NEA master$ _.rtn
1
```
