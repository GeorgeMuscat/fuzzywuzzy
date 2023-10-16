from abc import ABC
from typing import Iterator


class BaseMutation(ABC):
    def generate(self, sample: bytes) -> Iterator[bytes]:
        yield b""
