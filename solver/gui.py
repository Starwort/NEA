import pathlib
import platform
import subprocess
import sys
import threading
import tkinter as tk
import typing
from time import sleep

if typing.TYPE_CHECKING:
    from tkinter import ttk

import pygubu
from pyautogui import FailSafeException

from .automation import run_solution
from .constants import MOVE
from .typing import *
from .vision import identify_board

PROJECT_PATH = pathlib.Path(__file__).parent
PROJECT_UI = PROJECT_PATH / "solver.ui"


class SolverApp:
    def __init__(self, master=None):
        # Ugly hack to make pygubu work
        callback_strings = {
            "ui.text.timeout": (
                "How long to wait before stopping the solver, in seconds"
            ),
            "ui.text.blank": "",
            "ui.text.cheat": "Whether or not to allow the solver to Cheat",
            "ui.text.max": "Maximum number of moves to allow - should be a power of 2",
            "ui.text.min": "The starting depth the solver will use",
            "ui.text.games": "How many games to solve",
            "ui.text.cache": "How often to cache the board state",
            "ui.text.continue": (
                "How long to search for a more optimal solution, in milliseconds"
            ),
        }
        for key, string in callback_strings.items():
            setattr(self, f"setstatus({key})", self.setstatus(string))
        self.builder = builder = pygubu.Builder()
        builder.add_resource_path(PROJECT_PATH)
        builder.add_from_file(PROJECT_UI)
        self.mainwindow: "ttk.Frame" = builder.get_object("frame_1")
        builder.connect_callbacks(self)

        self.mainwindow.winfo_toplevel().title("MOLEK-SYNTEZ Solitaire Solver")
        self.status: "ttk.Label" = builder.get_object("status")
        self.allow_cheat: tk.BooleanVar = builder.get_variable("cheat")
        self.continue_milliseconds: tk.IntVar = builder.get_variable("continue_millis")
        self.cache_boundary: tk.IntVar = builder.get_variable("cache")
        self.maximum_moves: tk.IntVar = builder.get_variable("max_moves")
        self.minimum_moves: tk.IntVar = builder.get_variable("min_depth")
        self.solver_timeout: tk.DoubleVar = builder.get_variable("timeout")
        self.games: tk.IntVar = builder.get_variable("games")
        self.go: "ttk.Button" = builder.get_object("go")
        self.running = False
        self.set_status("")

    def setstatus(self, status: str) -> typing.Callable:
        if not status:
            status = "Hover over a label for more information"

        def callback(*_, **__):
            self.status.config(text=status)

        return callback

    def set_status(self, status: str) -> None:
        self.setstatus(status)()

    def solve(self):
        if not self.running:
            self.running = True
            threading.Thread(
                target=self.solve_n_games,
                kwargs=dict(
                    allow_cheat=self.allow_cheat.get(),
                    continue_milliseconds=self.continue_milliseconds.get(),
                    cache_boundary=self.cache_boundary.get(),
                    maximum_moves=self.maximum_moves.get(),
                    minimum_moves=self.minimum_moves.get(),
                    solver_timeout=self.solver_timeout.get(),
                ),
            ).start()
            self.go.config(text="Stop")
        else:
            self.running = False
            self.go.config(text="Go")

    def solve_n_games(self, **kwargs) -> None:
        """Solve n games automatically"""
        # I can't simply use a for-loop here because some games fail to solve
        n = self.games.get()
        solved = 0
        while solved < n and self.running:
            solution = self.full_solve_board(**kwargs)
            if solution:
                solved += 1
            try:
                run_solution(solution, solved != n)
            except FailSafeException:
                self.set_status("Aborted")
                return
            if solution:
                self.set_status("Waiting for deal" if solved != n else "")
            sleep(5)

    def run(self):
        self.mainwindow.mainloop()

    def full_solve_board(
        self,
        board: Image = None,
        allow_cheat: bool = True,
        continue_milliseconds: int = None,
        cache_boundary: int = None,
        maximum_moves: int = None,
        minimum_moves: int = None,
        solver_timeout: float = 10,
    ) -> List[Move]:
        self.set_status("Identifying board...")
        recognised_board = identify_board(board)
        return self.solve_board(
            recognised_board,
            allow_cheat=allow_cheat,
            continue_milliseconds=continue_milliseconds,
            cache_boundary=cache_boundary,
            maximum_moves=maximum_moves,
            minimum_moves=minimum_moves,
            solver_timeout=solver_timeout,
        )

    def solve_board(
        self,
        board: Board,
        allow_cheat: bool = True,
        continue_milliseconds: int = None,
        cache_boundary: int = None,
        maximum_moves: int = None,
        minimum_moves: int = None,
        print_stats: bool = True,
        solver_timeout: float = 10,
    ) -> List[Move]:
        root = pathlib.Path(__file__).parent.parent
        kernel = platform.system()
        if kernel == "Windows":
            bits, _ = platform.architecture()
            solver = root / "dist" / f"solver.{bits[:2]}.exe"
        elif kernel in ["Darwin", "Linux"]:
            self.set_status("Building solver for your OS...")
            try:
                subprocess.run("make", check=True)
            except subprocess.CalledProcessError:
                self.set_status("Failed to build the solver.")
                sleep(0.5)
            solver = root / "dist" / "solver"
        else:
            print(f"Unrecognised platform kernel: {kernel}", file=sys.stderr)
            self.set_status("ERROR: Could not determine appropriate executable")
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
        if minimum_moves is not None:
            options.append("-d")
            options.append(str(minimum_moves))
        try:
            self.set_status("Running the solver...")
            process = subprocess.run(
                [str(solver), *options, serialise_board(board)],
                shell=False,
                stdout=subprocess.PIPE,
                timeout=solver_timeout,
            )
            out = process.stdout.decode("ascii")
            if not out:
                self.set_status("No solution found!")
                return []
        except subprocess.TimeoutExpired:
            self.set_status("Solver timed out!")
            return []
        moves: List[Move] = []
        for line in out.splitlines():
            from_x, from_y, move_type, to_x, to_y = MOVE.match(line).groups()
            move = Move(
                int(from_x), int(from_y), int(to_x), int(to_y), move_type == "!"
            )
            moves.append(move)
        num_cheats = sum(move.cheat for move in moves)
        self.set_status(
            f"{len(moves)} moves, {num_cheats} Cheats - move your mouse to any corner"
            " to stop"
        )
        return moves


if __name__ == "__main__":
    import tkinter as tk

    root = tk.Tk()
    app = SolverApp(root)
    app.run()
