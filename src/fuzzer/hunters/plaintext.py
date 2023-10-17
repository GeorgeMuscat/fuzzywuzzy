from itertools import cycle
from typing import Iterator
from fuzzer.mutations.bitflip import flip_byte_mutation

from fuzzer.mutations.buffer_overflow import buffer_overflow_mutation
from fuzzer.mutations.known_integers import (
    known_integer_ascii_dec_mutation,
    known_integer_ascii_hex_mutation,
    known_integer_ascii_hex_with_prefix_mutation,
    known_integer_packed_be_mutation,
    known_integer_packed_le_mutation,
)

from ..utils import round_robin

MUTATORS = [
    buffer_overflow_mutation,
    known_integer_ascii_dec_mutation,
    known_integer_ascii_hex_mutation,
    known_integer_ascii_hex_with_prefix_mutation,
    known_integer_packed_be_mutation,
    known_integer_packed_le_mutation,
    flip_byte_mutation,
]


def whole_text_hunter(sample_input: bytes) -> Iterator[bytes]:
    """Runs each mutator on the entire sample_input."""
    for mutated_input in round_robin([mutator(sample_input) for mutator in MUTATORS]):
        yield mutated_input


def segment_hunter(sep: bytes):
    """Runs each mutator on each segment of the sample input individually, based on a predefined set of delimiters."""

    def hunter(sample_input: bytes) -> Iterator[bytes]:
        segments = sample_input.split(sep)
        for i, seg in enumerate(segments):
            for mutated_input in round_robin([mutator(seg) for mutator in MUTATORS]):
                yield sep.join(segments[:i] + [mutated_input] + segments[i + 1 :])

    return hunter
