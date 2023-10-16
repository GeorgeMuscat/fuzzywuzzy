from itertools import cycle
from typing import Iterator

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
]


def whole_text_hunter(sample_input: bytes) -> Iterator[bytes]:
    for mutated_input in round_robin([mutator(sample_input) for mutator in MUTATORS]):
        yield mutated_input


def line_hunter(sample_input: bytes) -> Iterator[bytes]:
    lines = sample_input.split(b"\n")
    for i, line in enumerate(lines):
        for mutated_input in round_robin([mutator(line) for mutator in MUTATORS]):
            yield b"\n".join(lines[:i] + [mutated_input] + lines[i + 1 :])
