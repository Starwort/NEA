import pyautogui  # type: ignore
from PIL import Image, ImageChops  # type: ignore
from .constants import *
from .typing import *


def identify_board(img: Image = None) -> Board:
    if img is None:
        img = pyautogui.screenshot()
    img = img.resize((img.width // SCALE, img.height // SCALE)).convert("1")
    return cast(
        Board,
        tuple(
            tuple(
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
                for x in range(6)
            )
            for y in range(6)
        ),
    )


def print_board(board: Optional[Board] = None) -> None:
    if board is None:
        board = identify_board()
    for row in board:
        print("{:>2}   {:>2}   {:>2}   {:>2}   {:>2}   {:>2}".format(*row))
