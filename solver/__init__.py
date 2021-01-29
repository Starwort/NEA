from time import sleep

from .automation import *
from .constants import *
from .solver import *
from .typing import *
from .vision import *


def solve_n_games(n: int, **kwargs) -> None:
    """Solve n games automatically"""
    # I can't simply use a for-loop here because some games fail to solve
    solved = 0
    while solved < n:
        solution = full_solve_board(**kwargs)
        if solution:
            solved += 1
        run_solution(solution, solved != n)
        sleep(5)
