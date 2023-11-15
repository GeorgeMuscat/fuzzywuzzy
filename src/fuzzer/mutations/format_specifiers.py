from typing import Iterator
from random import choice, randint

def random_format_specifiers(sample_input: bytes) -> Iterator[bytes]:
    """Ignores sample_input and outputs random format specifiers."""
    for _ in range(100):
        attack = b"%s%s%s"
        for _ in range(1000):
            attack += b"%" + str(randint(1, 100)).encode() + b"$" + choice([b"s", b"p", b"Lf"])
            yield attack
