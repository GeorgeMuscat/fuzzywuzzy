from typing import Iterator


def buffer_overflow_mutation(sample_input: bytes) -> Iterator[bytes]:
    for i in range(7, 20):
        yield b"A" * 2**i
