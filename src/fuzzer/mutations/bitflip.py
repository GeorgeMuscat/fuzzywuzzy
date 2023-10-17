from typing import Iterator

MASKS = [0xFF]


def flip_bits_mutation(sample_input: bytes) -> Iterator[bytes]:
    """Flips the bits!"""
    for mask in MASKS:
        for i in range(len(sample_input)):
            flipped_byte = sample_input[i] ^ mask
            list_of_bytes = list(sample_input)
            list_of_bytes[i] = flipped_byte
            yield bytes(list_of_bytes)
