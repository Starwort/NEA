from typing import *  # pylint: disable=unused-wildcard-import
from typing import cast, Final
from dataclasses import dataclass

__all__ = (
    "Row",
    "Board",
    "Cell",
    "Move",
    "List",
    "Tuple",
    "Optional",
    "cast",
    "Any",
    "Final",
    "Set",
    "Dict",
    "Union",
    "StateBoard",
)

Row = Tuple[str, str, str, str, str, str]
Board = Tuple[Row, Row, Row, Row, Row, Row]
Cell = Tuple[int, int]
