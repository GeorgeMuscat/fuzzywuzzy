from abc import ABC, abstractmethod
from pathlib import Path
from typing import TypedDict


class HarnessResult(TypedDict):
    duration: float
    exit_code: int
    events: list[tuple]


class Harness(ABC):
    @abstractmethod
    def __init__(self, binary_path: Path):
        pass

    @abstractmethod
    def run(self, input: bytes) -> HarnessResult:
        pass
