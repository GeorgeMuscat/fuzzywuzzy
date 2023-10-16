from itertools import cycle
from typing import Iterator

from ..utils import round_robin

MUTATORS = []


def whole_text_hunter(sample_input: bytes) -> Iterator[bytes]:
    for mutator in round_robin([mutator(sample_input) for mutator in MUTATORS]):
        yield next(mutator)
