from typing import Iterator


def flip_byte_mutation(sample_input: bytes) -> Iterator[bytes]:
    """Flips one byte at a time!"""
    for mask in range(0x100):
        for i in range(len(sample_input)):
            flipped_byte = sample_input[i] ^ mask
            list_of_bytes = list(sample_input)
            list_of_bytes[i] = flipped_byte
            yield bytes(list_of_bytes)
