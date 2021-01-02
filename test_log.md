# Test log

This will log the tests, their output, and whether the test was successful

## Test #1: Board State Parsing

[Commit of test (used to view the test at the time)](https://github.com/Starwort/NEA/commit/ee750d115fd0a794eb092f42938e4e3377302c9c)

```output
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

```output
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

```output
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

## Test #4: Board state permutation invariance

[Commit of test (used to view the test at the time)](https://github.com/Starwort/NEA/commit/c7bc22b7a4cac4e2709f71ed49e2df7cbbedc77b)

Output: [![asciicast](https://asciinema.org/a/2JbOBIMBh1DxGX0zAnPkRYNl3.svg)](https://asciinema.org/a/2JbOBIMBh1DxGX0zAnPkRYNl3)

```output
starwort@hedwig ~/Documents/NEA master$ bash -c '(read a; while read b; do dist/test \"$a\" \"$b\"; done) < samples/permutations.txt' | grep -B 2 false
```

(Author's note: I use [xonsh](https://xon.sh/) so the bash snippet needs to be emulated in bash)

Test result: Successful; All permutations of the board state are accepted as equivalent in comparison
