from dataclasses import dataclass
from typing import Dict  # included for 3.8- compatibility
from typing import List  # included for 3.8- compatibility
from typing import Set  # included for 3.8- compatibility
from typing import Tuple  # included for 3.8- compatibility
from typing import Any, Final, Optional, Union, cast

from PIL.Image import Image

__all__ = (
    "Any",
    "Board",
    "Dict",
    "Final",
    "Image",
    "Board",
    "Column",
    "List",
    "Move",
    "Optional",
    "Set",
    "Tuple",
    "Union",
    "cast",
    "serialise_board",
)


@dataclass
class Move:
    from_x: int
    from_y: int
    to_x: int
    to_y: int
    cheat: bool


Column = Tuple[List[str], bool]
Board = Tuple[
    Column,
    Column,
    Column,
    Column,
    Column,
    Column,
]


def serialise_board(board: Board) -> str:
    return "".join("".join(cards) + ".!"[cheat] for cards, cheat in board)
