import json
from json import JSONDecodeError
from typing import Iterator
from random import choice

from ..utils import round_robin
from fuzzer.mutations.buffer_overflow import buffer_overflow_mutation
from fuzzer.mutations.known_integers import (
    known_integer_ascii_dec_mutation,
    known_integer_ascii_hex_mutation,
    known_integer_ascii_hex_with_prefix_mutation,
    known_integer_packed_be_mutation,
    known_integer_packed_le_mutation,
    known_integer_nearby,
)
from fuzzer.mutations.bitflip import flip_byte_mutation
from fuzzer.mutations.numbers import (
    close_number_mutation,
    rand_number_mutation
)

from fuzzer.mutations.lists import (
    add_to_list_mutator,
    long_items_list_mutator,
)

MUTATORS = [
    buffer_overflow_mutation,
    known_integer_ascii_dec_mutation,
    known_integer_ascii_hex_mutation,
    known_integer_ascii_hex_with_prefix_mutation,
    known_integer_packed_be_mutation,
    known_integer_packed_le_mutation,
    flip_byte_mutation,
]

NUMBER_MUTATORS = [
    close_number_mutation,
    rand_number_mutation,
    known_integer_nearby,
]

LIST_MUTATORS = [
    add_to_list_mutator,
    long_items_list_mutator,
]

def json_key_hunter(sample_input: bytes) -> Iterator[bytes]:
    """Runs each mutator on the keys of the sample_input, interpreted as json."""
    try:
        parsed_json = json.loads(sample_input.decode().replace("'", '"'))
    except JSONDecodeError:
        return
    for key in parsed_json.keys():
        for mutated_key in round_robin([mutator(key.encode()) for mutator in MUTATORS]):
            mutated_json = parsed_json.copy()
            try:
                mutated_json[mutated_key.decode()] = mutated_json[key]
            except UnicodeDecodeError:
                continue
            del mutated_json[key]
            yield json.dumps(mutated_json).encode()


def json_number_value_hunter(sample_input: bytes) -> Iterator[bytes]:
    """Runs each mutator on the values of the sample_input, interpreted as json."""
    try:
        parsed_json = json.loads(sample_input.decode().replace("'", '"'))
    except JSONDecodeError:
        return
    for key, value in parsed_json.items():
        mutated_json = parsed_json.copy()
        if not isinstance(value, int):
            continue
        for mutated_value in round_robin([mutator(value) for mutator in NUMBER_MUTATORS]):
            try:
                mutated_json[key] = mutated_value
            except UnicodeDecodeError:
                continue
            yield json.dumps(mutated_json).encode()


def json_array_hunter(sample_input: bytes) -> Iterator[bytes]:
    """Runs each mutator on the values of the sample_input, interpreted as json."""
    try:
        parsed_json = json.loads(sample_input.decode().replace("'", '"'))
    except JSONDecodeError:
        return
    for key, value in parsed_json.items():
        mutated_json = parsed_json.copy()
        if not isinstance(value, list):
            continue
        for mutated_value in round_robin([mutator(value) for mutator in LIST_MUTATORS]):
            try:
                mutated_json[key] = mutated_value
            except UnicodeDecodeError:
                continue
            yield json.dumps(mutated_json).encode()

def json_value_hunter(sample_input: bytes) -> Iterator[bytes]:
    """Runs each mutator on the values of the sample_input, interpreted as json."""
    try:
        parsed_json = json.loads(sample_input.decode().replace("'", '"'))
    except JSONDecodeError:
        return
    for key, value in parsed_json.items():
        mutated_json = parsed_json.copy()
        for mutated_value in round_robin([mutator(str(value).encode()) for mutator in MUTATORS]):
            try:
                mutated_json[key] = mutated_value.decode()
            except UnicodeDecodeError:
                continue
            yield json.dumps(mutated_json).encode()

def json_key_remover(sample_input: bytes) -> Iterator[bytes]:
    """Just removes a key value pair from the json"""
    try:
        parsed_json = json.loads(sample_input.decode().replace("'", '"'))
    except JSONDecodeError:
        return
    for key in parsed_json:
        mutated_json = parsed_json.copy()
        del mutated_json[key]
        yield json.dumps(mutated_json).encode()