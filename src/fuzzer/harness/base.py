from abc import ABC, abstractmethod
from pathlib import Path
from typing import Optional, TypedDict


class HarnessResult(TypedDict):
    duration: float
    exit_code: Optional[int]
    events: list[tuple]


class BaseHarness(ABC):
    TIMEOUT = 1

    @abstractmethod
    def __init__(self, binary_path: Path, debug: bool = False):
        pass

    @abstractmethod
    def run(self, input: bytes) -> HarnessResult:
        pass

    def set_debug(self, debug: bool):
        pass
