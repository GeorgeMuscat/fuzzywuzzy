from typing import Iterator


def buffer_overflow_mutation(sample_input: bytes) -> Iterator[bytes]:
    """Ignores sample_input and generates buffer overflows."""
    for i in range(7, 18):
        yield b"A" * 2**i
