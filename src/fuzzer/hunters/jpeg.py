from struct import unpack

from fuzzer.utils import round_robin
from ..mutations.keywords import delete_keywords, repeat_keywords


MUTATORS = [delete_keywords, repeat_keywords]

MARKERS = {
    0xFFD8.to_bytes(2, "big"): "Start of Image",
    0xFFE0.to_bytes(2, "big"): "Application Default Header",
    0xFFDB.to_bytes(2, "big"): "Quantization Table",
    0xFFC0.to_bytes(2, "big"): "Start of Frame",
    0xFFC4.to_bytes(2, "big"): "Define Huffman Table",
    0xFFDA.to_bytes(2, "big"): "Start of Scan",
    0xFFD9.to_bytes(2, "big"): "End of Image",
}

# https://en.wikibooks.org/wiki/JPEG_-_Idea_and_Practice/The_header_part


def marker_hunter(sample_input: bytes):
    """
    Hunts all occurances of a marker
    """
    i = 0
    for mutated_input in round_robin(
        [mutator(sample_input, list(MARKERS.keys())) for mutator in MUTATORS]
    ):
        i += 1
        file = open("test_" + str(i), "wb")
        file.write(mutated_input)
        yield mutated_input
