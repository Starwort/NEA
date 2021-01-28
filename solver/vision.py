from itertools import zip_longest

import pyautogui  # type: ignore
from PIL import Image, ImageChops  # type: ignore

from .constants import (
    CARD_HEIGHT,
    CARD_WIDTH,
    CARDS,
    COL_WIDTH,
    GAME_RESOLUTION,
    LEFT,
    ROW_HEIGHT,
    TOP,
)
from .typing import *

try:
    import pygetwindow

    def take_screenshot() -> Image:
        try:
            # Find the game window
            game_window = pygetwindow.getWindowsWithTitle("MOLEK-SYNTEZ")[0]
            # Focus the window (so we can manipulate it)
            game_window.activate()
            # Find out how big the window is
            crop_vector = game_window.box
            # Take a screenshot
            img = pyautogui.screenshot()
            # Crop and return it
            return img.crop(crop_vector)
        except IndexError:
            # Couldn't find the window which probably means I made a mistake
            return pyautogui.screenshot()

    def get_bounds_and_scale() -> Tuple[Tuple[int, int], int]:
        """Return the real coordinates and scale of the window"""
        try:
            # Find the game window
            game_window = pygetwindow.getWindowsWithTitle("MOLEK-SYNTEZ")[0]
            # Focus the window (so we can manipulate it)
            game_window.activate()
            # Find out how big the window is
            game_bounds = game_window.box
            # Calculate its height and width
            width = game_bounds[2] - game_bounds[0]
            height = game_bounds[3] - game_bounds[1]

            # Find out how many times the game has been scaled up; note that the
            # fullscreen window gets a surrounding black border and so the game will
            # be scaled as many times as it can fit and then centred
            scale = min(width // GAME_RESOLUTION[0], height // GAME_RESOLUTION[1])
            new_width, new_height = (
                GAME_RESOLUTION[0] * scale,
                GAME_RESOLUTION[1] * scale,
            )

            # Get the top left corner of the image centre
            left = game_bounds[0] + (width - new_width) / 2
            top = game_bounds[1] + (height - new_height) / 2
            return (left, top), scale
        except IndexError:
            # Couldn't find the window which probably means I made a mistake
            # Fall back to the 'pygetwindow missing' algorithm
            img = pyautogui.screenshot()
            # Find out how many times the game has been scaled up; note that the
            # fullscreen window gets a surrounding black border and so the game will
            # be scaled as many times as it can fit and then centred
            scale = min(
                img.width // GAME_RESOLUTION[0], img.height // GAME_RESOLUTION[1]
            )
            new_width, new_height = (
                GAME_RESOLUTION[0] * scale,
                GAME_RESOLUTION[1] * scale,
            )

            # Get the top left corner of the image centre
            left = (img.width - new_width) / 2
            top = (img.height - new_height) / 2
            return (left, top), scale


except (NotImplementedError, ImportError):

    def take_screenshot() -> Image:
        return pyautogui.screenshot()

    def get_bounds_and_scale() -> Tuple[Tuple[int, int], int]:
        """Return the real coordinates and scale of the window"""
        img = pyautogui.screenshot()
        # Find out how many times the game has been scaled up; note that the
        # fullscreen window gets a surrounding black border and so the game will
        # be scaled as many times as it can fit and then centred
        scale = min(img.width // GAME_RESOLUTION[0], img.height // GAME_RESOLUTION[1])
        new_width, new_height = GAME_RESOLUTION[0] * scale, GAME_RESOLUTION[1] * scale

        # Get the bounds of the image centre
        left = (img.width - new_width) / 2
        top = (img.height - new_height) / 2
        return (left, top), scale


def identify_board(img: Image = None) -> Board:
    """OCR the (new) board represented in img.

    If img is None, a screenshot of the primary monitor - or, when
    available, the game window - is taken.

    img must represent a new game with the window in pixel-perfect scaling
    mode and contain only the contents of the game window
    """
    if img is None:
        img = pyautogui.screenshot()
    # Find out how many times the game has been scaled up; note that the
    # fullscreen window gets a surrounding black border and so the game will
    # be scaled as many times as it can fit and then centred
    scale = min(img.width // GAME_RESOLUTION[0], img.height // GAME_RESOLUTION[1])
    new_width, new_height = GAME_RESOLUTION[0] * scale, GAME_RESOLUTION[1] * scale

    # Get the bounds of the image centre
    left = (img.width - new_width) / 2
    top = (img.height - new_height) / 2
    right = (img.width + new_width) / 2
    bottom = (img.height + new_height) / 2

    # crop then resize the image
    img = img.crop((left, top, right, bottom))
    img = img.resize(GAME_RESOLUTION).convert("1")
    return cast(
        Board,
        tuple(
            (
                [
                    (
                        [
                            card
                            for card, card_img in CARDS.items()
                            if not ImageChops.difference(
                                card_img,
                                img.crop(
                                    (
                                        LEFT + COL_WIDTH * x,
                                        TOP + ROW_HEIGHT * y,
                                        LEFT + COL_WIDTH * x + CARD_WIDTH,
                                        TOP + ROW_HEIGHT * y + CARD_HEIGHT,
                                    )
                                ),
                            ).getbbox()
                        ]
                        + ["?"]
                    )[0]
                    for y in range(6)
                ],
                False,
            )
            for x in range(6)
        ),
    )


def print_board(board: Optional[Board] = None, **kw) -> None:
    """Print a board"""
    if board is None:
        board = identify_board()
    for row in zip_longest(*(cards for cards, cheat in board), fillvalue=""):
        print("{:>2}   {:>2}   {:>2}   {:>2}   {:>2}   {:>2}".format(*row), **kw)
