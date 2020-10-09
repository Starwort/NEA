from typing import Final
from PIL import Image  # type: ignore

SCALE: Final = 2
CARD_WIDTH: Final = 8
CARD_HEIGHT: Final = 9
LEFT: Final = 244
TOP: Final = 149
COL_WIDTH: Final = 82
ROW_HEIGHT: Final = 16
CARDS: Final = {
    "T": Image.open("solitaire/samples/T.png"),
    "K": Image.open("solitaire/samples/K.png"),
    "D": Image.open("solitaire/samples/D.png"),
    "V": Image.open("solitaire/samples/V.png"),
    "10": Image.open("solitaire/samples/10.png"),
    "9": Image.open("solitaire/samples/9.png"),
    "8": Image.open("solitaire/samples/8.png"),
    "7": Image.open("solitaire/samples/7.png"),
    "6": Image.open("solitaire/samples/6.png"),
}
