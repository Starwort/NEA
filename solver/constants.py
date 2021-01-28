import re
from typing import Dict, Final

from PIL import Image  # type: ignore

MOVE = re.compile(r"([0-5]) (\d+) (!|-)> ([0-5]) (\d+)")
GAME_RESOLUTION: Final = (960, 540)
CARD_WIDTH: Final = 8
CARD_HEIGHT: Final = 9
LEFT: Final = 244
TOP: Final = 149
COL_WIDTH: Final = 82
ROW_HEIGHT: Final = 16
NEW_GAME = (482, 517)
CARDS: Final[Dict[str, Image.Image]] = {
    "T": Image.open("samples/T.png"),
    "K": Image.open("samples/K.png"),
    "D": Image.open("samples/D.png"),
    "V": Image.open("samples/V.png"),
    "0": Image.open("samples/10.png"),
    "9": Image.open("samples/9.png"),
    "8": Image.open("samples/8.png"),
    "7": Image.open("samples/7.png"),
    "6": Image.open("samples/6.png"),
}
