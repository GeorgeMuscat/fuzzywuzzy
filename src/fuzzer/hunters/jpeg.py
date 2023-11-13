from pickle import MARK
from struct import unpack

from fuzzer.utils import round_robin
from ..mutations.keywords import delete_keywords, repeat_keywords
from ..mutations.bitflip import flip_byte_mutation

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
    HEADER_MUTATORS = [flip_byte_mutation]
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
        yield before_header + mutated_header + after_header
