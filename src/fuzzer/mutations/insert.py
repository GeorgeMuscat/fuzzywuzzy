import random
from typing import Iterator


def random_insert_null_mutation(sample_input: bytes) -> Iterator[bytes]:
    r = random.Random()

    for i in range(0, 10):
        n = r.randint(0, len(sample_input))
        yield sample_input[:n] + b"\0" + sample_input[n:]
