import json
from typing import Iterator

from ..utils import round_robin
from fuzzer.mutations.buffer_overflow import buffer_overflow_mutation
from fuzzer.mutations.known_integers import (
    known_integer_ascii_dec_mutation,
    known_integer_ascii_hex_mutation,
    known_integer_ascii_hex_with_prefix_mutation,
    known_integer_packed_be_mutation,
    known_integer_packed_le_mutation,
)
from fuzzer.mutations.bitflip import flip_byte_mutation

MUTATORS = [
    buffer_overflow_mutation,
    known_integer_ascii_dec_mutation,
    known_integer_ascii_hex_mutation,
    known_integer_ascii_hex_with_prefix_mutation,
    known_integer_packed_be_mutation,
    known_integer_packed_le_mutation,
    flip_byte_mutation,
]

def json_key_hunter(sample_input: bytes) -> Iterator[bytes]:
    """Runs each mutator on the keys of the sample_input, interpreted as json."""
    parsed_json = json.loads(sample_input.decode().replace("'", '"'))
    for key in parsed_json.keys():
        for mutated_key in round_robin([mutator(key.encode()) for mutator in MUTATORS]):
            mutated_json = parsed_json.copy()
            try:
                mutated_json[mutated_key.decode()] = mutated_json[key]
            except UnicodeDecodeError:
                continue
            del mutated_json[key]
            yield json.dumps(mutated_json).encode()

def json_value_hunter(sample_input: bytes) -> Iterator[bytes]:
    """Runs each mutator on the values of the sample_input, interpreted as json."""
    parsed_json = json.loads(sample_input.decode().replace("'", '"'))
    for key, value in parsed_json.items():
        mutated_json = parsed_json.copy()
        for mutated_value in round_robin([mutator(str(value).encode()) for mutator in MUTATORS]):
            try:
                mutated_json[key] = mutated_value.decode()
            except UnicodeDecodeError:
                continue
            yield json.dumps(mutated_json).encode()