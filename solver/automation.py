import pyautogui

from .constants import COL_WIDTH, LEFT, NEW_GAME, ROW_HEIGHT, TOP
from .typing import *
from .vision import get_bounds_and_scale


def click(x: int, y: int) -> None:
    """Click at (x, y)

    pyautogui.click is too fast and clicks aren't registering
    """
    pyautogui.moveTo(x, y)
    pyautogui.mouseDown(x, y)
    pyautogui.mouseUp(x, y)


def run_solution(solution: List[Move], start_new_game: bool = True) -> None:
    """Execute the given solution in the game, using automated mouse movements"""
    (left, top), scale = get_bounds_and_scale()
    for move in solution:
        real_from_y = top + (TOP + ROW_HEIGHT * move.from_y) * scale
        real_from_x = left + (LEFT + COL_WIDTH * move.from_x) * scale
        real_to_y = top + (TOP + ROW_HEIGHT * move.to_y) * scale
        real_to_x = left + (LEFT + COL_WIDTH * move.to_x) * scale
        click(real_from_x, real_from_y)
        click(real_to_x, real_to_y)
    if start_new_game:
        click(left + NEW_GAME[0] * scale, top + NEW_GAME[1] * scale)
