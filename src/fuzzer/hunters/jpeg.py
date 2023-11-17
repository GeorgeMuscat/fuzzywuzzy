from typing import Iterator

from fuzzer.utils import round_robin

from ..mutations.keywords import delete_keywords, repeat_keywords
from ..mutations.bitflip import flip_byte_mutation
from ..mutations.repeated_parts import repeat_segment
from ..mutations.known_integers import known_bytes_mutation

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


def region_hunter(from_marker: bytes, to_marker: bytes):
    MUTATORS = [flip_byte_mutation, repeat_segment, known_bytes_mutation]

    def inner(sample_input: bytes) -> Iterator[bytes]:
        before_region = sample_input.split(from_marker)[0] + from_marker
        after_region = to_marker + to_marker.join(sample_input.split(to_marker)[1:])
        region = sample_input[len(before_region) : len(sample_input) - len(after_region)]

        for mutated_region in round_robin([mutator(region) for mutator in MUTATORS]):
            yield before_region + mutated_region + after_region
    
    return inner


def marker_hunter(sample_input: bytes):
    """
    Hunts all occurances of a marker
    """
    MUTATORS = [delete_keywords, repeat_keywords]

    for mutated_input in round_robin(
        [mutator(sample_input, list(MARKERS.values())) for mutator in MUTATORS]
    ):
        yield mutated_input


def header_hunter(sample_input: bytes):
    """Gets the header and performs mutations on it"""
    ADH = MARKERS["Application Default Header"]
    DQT = MARKERS["Quantization Table"]
    
    for mutation in region_hunter(ADH, DQT)(sample_input):
        yield mutation


def quantization_table_hunter(sample_input: bytes) -> Iterator[bytes]:
    DQT = MARKERS["Quantization Table"]
    SOF = MARKERS["Start of Frame"]
    
    for mutation in region_hunter(DQT, SOF)(sample_input):
        yield mutation


def frame_hunter(sample_input: bytes) -> Iterator[bytes]:
    SOF = MARKERS["Start of Frame"]
    DHT = MARKERS["Define Huffman Table"]
    
    for mutation in region_hunter(SOF, DHT)(sample_input):
        yield mutation


def huffman_hunter(sample_input: bytes) -> Iterator[bytes]:
    DHT = MARKERS["Define Huffman Table"]
    SOS = MARKERS["Start of Scan"]
    
    for mutation in region_hunter(DHT, SOS)(sample_input):
        yield mutation

def image_hunter(sample_input: bytes) -> Iterator[bytes]:
    SOS = MARKERS["Start of Scan"]
    EOI = MARKERS["End of Image"]
    
    for mutation in region_hunter(SOS, EOI)(sample_input):
        yield mutation
