from pickle import MARK
from struct import unpack
from fuzzer.mutations import buffer_overflow
from fuzzer.mutations.known_integers import known_integer_packed_be_mutation
from fuzzer.mutations.repeated_parts import repeat_segment

from fuzzer.utils import round_robin
from ..mutations.keywords import delete_keywords, repeat_keywords
from ..mutations.bitflip import flip_byte_mutation
from ..mutations.buffer_overflow import buffer_overflow_mutation

MUTATORS = [delete_keywords, repeat_keywords]

MARKERS = {
    "Start of Image": 0xFFD8.to_bytes(2, "big"),
    "Application Default Header": 0xFFE0.to_bytes(2, "big"),
    "Quantization Table": 0xFFDB.to_bytes(2, "big"),
    "Start of Frame": 0xFFC0.to_bytes(2, "big"),
    "Define Huffman Table": 0xFFC4.to_bytes(2, "big"),
    "Start of Scan": 0xFFDA.to_bytes(2, "big"),
    "End of Image": 0xFFD9.to_bytes(2, "big"),
}
# I think im actually just wrong here... https://www.media.mit.edu/pia/Research/deepview/exif.html

# https://en.wikibooks.org/wiki/JPEG_-_Idea_and_Practice/The_header_part


def marker_hunter(sample_input: bytes):
    """
    Hunts all occurances of a marker
    """
    # i = 0
    for mutated_input in round_robin(
        [mutator(sample_input, list(MARKERS.values())) for mutator in MUTATORS]
    ):
        # i += 1
        # file = open("test_" + str(i), "wb")
        # file.write(mutated_input)
        yield mutated_input


def header_hunter(sample_input: bytes):
    """
    Gets the header and performs mutations on it
    """
    HEADER_MUTATORS = [flip_byte_mutation, buffer_overflow_mutation, repeat_segment]
    # Get the header (after 0xFFE0 and before 0xFFDB)
    after_header = MARKERS["Quantization Table"] + b"".join(
        sample_input.split(MARKERS["Quantization Table"])[1:]
    )
    header = sample_input.split(MARKERS["Quantization Table"])[0]
    before_header = (
        sample_input.split(MARKERS["Application Default Header"])[0]
        + MARKERS["Application Default Header"]
    )
    for mutated_header in round_robin([mutator(header) for mutator in HEADER_MUTATORS]):
        yield mutated_header + after_header


def quantization_table_hunter(sample_input: bytes):
    HEADER_MUTATORS = [flip_byte_mutation, buffer_overflow_mutation, repeat_segment]
    # Get the header (after 0xFFE0 and before 0xFFDB)
    after_table = MARKERS["Start of Frame"] + b"".join(
        sample_input.split(MARKERS["Start of Frame"])[1:]
    )
    table = sample_input.split(MARKERS["Start of Frame"])[0]
    before_table = (
        sample_input.split(MARKERS["Quantization Table"])[0]
        + MARKERS["Quantization Table"]
    )
    for mutated_table in round_robin([mutator(table) for mutator in HEADER_MUTATORS]):
        # print(mutated_header)
        yield before_table + mutated_table + after_table
