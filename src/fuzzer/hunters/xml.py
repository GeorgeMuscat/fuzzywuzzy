import xml.etree.ElementTree as xml
from typing import Iterator

from ..utils import round_robin
from fuzzer.mutations.format_specifiers import random_format_specifiers
from fuzzer.mutations.buffer_overflow import buffer_overflow_mutation
from fuzzer.mutations.known_integers import (
    known_integer_ascii_dec_mutation,
    known_integer_ascii_hex_mutation,
    known_integer_ascii_hex_with_prefix_mutation,
)

MUTATORS = [
    buffer_overflow_mutation,
    known_integer_ascii_dec_mutation,
    known_integer_ascii_hex_mutation,
    known_integer_ascii_hex_with_prefix_mutation,
    random_format_specifiers,
]

def xml_attribute_hunter(sample_input: bytes) -> Iterator[bytes]:
    """For every node in the xml tree, try fuzzing the attributes"""
    root = xml.fromstring(sample_input)
    for el in root.iter():
        for mutation in round_robin([mutator('') for mutator in MUTATORS]):
            for attr in el.attrib:
                el.set(attr, mutation.decode())
                yield xml.tostring(root)

def xml_text_hunter(sample_input: bytes) -> Iterator[bytes]:
    """For every node in the xml tree, try fuzzing the text field"""
    root = xml.fromstring(sample_input)
    for el in root.iter():
        for mutation in round_robin([mutator('') for mutator in MUTATORS]):
            el.text = mutation.decode()
            yield xml.tostring(root)

def xml_tag_hunter(sample_input: bytes) -> Iterator[bytes]:
    """For every node in the xml tree, try fuzzing the text field"""
    root = xml.fromstring(sample_input)
    for el in root.iter():
        for mutation in round_robin([mutator('') for mutator in MUTATORS]):
            el.tag = mutation.decode()
            yield xml.tostring(root)
