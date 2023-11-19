from typing import Iterator

from fuzzer.mutations.bitflip import flip_byte_mutation
from fuzzer.mutations.buffer_overflow import buffer_overflow_mutation
from fuzzer.mutations.insert import random_insert_null_mutation
from fuzzer.mutations.known_integers import (
    known_integer_ascii_dec_mutation,
    known_integer_ascii_hex_mutation,
    known_integer_ascii_hex_with_prefix_mutation,
    known_integer_packed_be_mutation,
    known_integer_packed_le_mutation,
)
from fuzzer.mutations.repeated_parts import repeat_last_segment_mutation
from fuzzer.mutations.format_specifiers import random_format_specifiers

from ..utils import round_robin, tag

MUTATORS = [
    buffer_overflow_mutation,
    known_integer_ascii_dec_mutation,
    known_integer_ascii_hex_mutation,
    known_integer_ascii_hex_with_prefix_mutation,
    known_integer_packed_be_mutation,
    known_integer_packed_le_mutation,
    flip_byte_mutation,
    random_insert_null_mutation,
    repeat_last_segment_mutation,
    random_format_specifiers
]


def whole_text_hunter(sample_input: bytes) -> Iterator[bytes]:
    """Runs each mutator on the entire sample_input."""
    for mutated_input in round_robin([mutator(sample_input) for mutator in MUTATORS]):
        yield mutated_input


def segment_hunter(sep: bytes):
    """Runs each mutator on each segment of the sample input individually, based on a predefined set of delimiters."""

    def hunter(sample_input: bytes) -> Iterator[bytes]:
        segments = sample_input.split(sep)

        # Creates an iterator that round robins over each segment, which is in turn round robin-ing over the mutators.
        iterator: Iterator[tuple[int, bytes]] = round_robin(
            [
                # `tag` adds a fixed value to each iterator value so we know which segment a mutation is for.
                round_robin([tag(i, mutator(seg)) for mutator in MUTATORS])
                for i, seg in enumerate(segments)
            ]
        )

        for i, mutation in iterator:
            yield sep.join(segments[:i] + [mutation] + segments[i + 1 :])

    return hunter
