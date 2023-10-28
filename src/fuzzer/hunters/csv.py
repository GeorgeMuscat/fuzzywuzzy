from typing import Iterator
from fuzzer.mutations.buffer_overflow import buffer_overflow_mutation

import fuzzer.mutations.repeated_parts as rep
import fuzzer.mutations.bitflip as bf

from ..utils import round_robin

MUTATORS = [rep.repeat_segment, rep.overwrite_segment, bf.flip_byte_mutation]


def whole_text_hunter(sample_input: bytes) -> Iterator[bytes]:
    """Runs each mutator on the entire sample_input."""
    for mutated_input in round_robin([mutator(sample_input) for mutator in MUTATORS]):
        yield mutated_input


def header_hunter(sample_input: bytes) -> Iterator[bytes]:
    original = sample_input.split(b"\n")
    header = original[0]
    for mutated_header in round_robin(
        [
            mutator(header)
            for mutator in [rep.overwrite_segment_comma, bf.flip_byte_mutation]
        ]
    ):
        copy = original.copy()
        copy[0] = mutated_header
        yield b"\n".join(copy)


def line_hunter(sample_input: bytes) -> Iterator[bytes]:
    """
    Hunts for a line, performs mutations on that hunted line.
    """
    inp = sample_input.split(b"\n")
    for i in range(len(inp)):
        for mutated_line in round_robin(
            [
                mutator(inp[i])
                for mutator in [
                    rep.overwrite_segment_comma,
                    bf.flip_byte_mutation,
                    buffer_overflow_mutation,
                ]
            ]
        ):
            copy = inp.copy()
            copy[i] = mutated_line
            yield b"\n".join(copy)
