import pathlib
import platform
import subprocess
import sys

from .constants import MOVE
from .typing import *
from .vision import identify_board
from .vision import print_board as display


def full_solve_board(
    board: Image = None,
    print_board: bool = True,
    allow_cheat: bool = True,
    continue_milliseconds: int = None,
    cache_boundary: int = None,
    maximum_moves: int = None,
    print_stats: bool = True,
    solver_timeout: float = 10,
) -> List[Move]:
    """Recognise and solve a board with the given solver flags

    If print_board is set, print the recognised board before solving it"""
    recognised_board = identify_board(board)
    if print_board:
        print(serialise_board(recognised_board))
        display(recognised_board)
        print()
    return solve_board(
        recognised_board,
        allow_cheat=allow_cheat,
        continue_milliseconds=continue_milliseconds,
        cache_boundary=cache_boundary,
        maximum_moves=maximum_moves,
        print_stats=print_stats,
        solver_timeout=solver_timeout,
    )


def solve_board(
    board: Board,
    allow_cheat: bool = True,
    continue_milliseconds: int = None,
    cache_boundary: int = None,
    maximum_moves: int = None,
    print_stats: bool = True,
    solver_timeout: float = 10,
) -> List[Move]:
    """Solve board with the given solver flags"""
    root = pathlib.Path(__file__).parent.parent
    kernel = platform.system()
    if kernel == "Windows":
        bits, _ = platform.architecture()
        solver = root / "dist" / f"solver.{bits[:2]}.exe"
    elif kernel in ["Darwin", "Linux"]:
        print("Building solver for your OS...")
        try:
            subprocess.run("make", check=True)
        except subprocess.CalledProcessError:
            print("Failed to build the solver.")
            if input("Continue? [Y]/*\n>>> ") not in ["", "y", "Y"]:
                return []
        solver = root / "dist" / "solver"
    else:
        print(f"Unrecognised platform kernel: {kernel}")
        print("Could not determine appropriate executable")
        return []
    options = []
    if allow_cheat:
        options.append("-c")
    if continue_milliseconds is not None:
        options.append("-t")
        options.append(str(continue_milliseconds))
    if cache_boundary is not None:
        options.append("-n")
        options.append(str(cache_boundary))
    if maximum_moves is not None:
        options.append("-m")
        options.append(str(maximum_moves))
    try:
        process = subprocess.run(
            [str(solver), serialise_board(board)] + options,
            capture_output=True,
            timeout=solver_timeout,
        )
        out = process.stdout.decode("ascii")
        if not out:
            # Sometimes the program fails to capture the output. I don't
            # know why, but let's just pretend no solution is found even if
            # it's a output-capture bug :D
            print("No solution found!", file=sys.stderr)
            return []
    except subprocess.TimeoutExpired:
        print("Solver timed out!", file=sys.stderr)
        return []
    moves: List[Move] = []
    for line in out.splitlines():
        from_x, from_y, move_type, to_x, to_y = MOVE.match(line).groups()
        move = Move(int(from_x), int(from_y), int(to_x), int(to_y), move_type == "!")
        moves.append(move)
    if print_stats:
        print("Moves:", len(moves))
        num_cheats = sum(move.cheat for move in moves)
        print("Number of Cheats:", num_cheats)
    return moves
