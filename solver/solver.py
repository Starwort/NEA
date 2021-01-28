import pathlib
import platform
import subprocess

from .constants import MOVE
from .typing import *
from .vision import identify_board
from .vision import print_board as display


def full_solve_board(
    board: Image = None,
    print_board: bool = True,
    allow_cheat: bool = False,
    continue_milliseconds: int = None,
    cache_boundary: int = None,
    maximum_moves: int = None,
) -> List[Move]:
    """Recognise and solve a board with the given solver flags

    If print_board is set, print the recognised board before solving it"""
    recognised_board = identify_board(board)
    if print_board:
        display(recognised_board)
    return solve_board(
        recognised_board,
        allow_cheat=allow_cheat,
        continue_milliseconds=continue_milliseconds,
        cache_boundary=cache_boundary,
        maximum_moves=maximum_moves,
    )


def solve_board(
    board: Board = None,
    allow_cheat: bool = False,
    continue_milliseconds: int = None,
    cache_boundary: int = None,
    maximum_moves: int = None,
) -> List[Move]:
    """Solve board with the given solver flags"""
    root = pathlib.Path(__file__).parent.parent
    kernel = platform.system()
    if kernel == "Windows":
        bits, _ = platform.architecture()
        solver = root / "dist" / f"solver.{bits}.exe"
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
    process = subprocess.run(
        [str(solver), serialise_board(board)] + options, capture_output=True
    )
    out = process.stdout
    moves = []
    for line in out.decode("ascii").splitlines():
        from_x, from_y, move_type, to_x, to_y = MOVE.match(line).groups()
        move = Move(int(from_x), int(from_y), int(to_x), int(to_y), move_type == "!")
        moves.append(move)
    return moves
