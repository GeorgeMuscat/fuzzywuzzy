import random
from typing import Iterator


def close_number_mutation(number) -> Iterator[int]:
    for i in range(0, 1000):
        yield number + i
        yield number - i


def rand_number_mutation(number) -> Iterator[int]:
    for i in range(0, 1000):
        yield random.randint(-2147483648, 2147483648)