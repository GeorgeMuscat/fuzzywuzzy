from typing import Iterator

INTS = [
    0xFFFFFFFF,
    0x0,
    0x1,
    -0x1,
    0x80000000,
    0x7FFFFFFF,
    *(2**i for i in range(1, 20)),
    *(2**i + 1 for i in range(1, 20)),
]


def known_integer_packed_le_mutation(sample_input: bytes) -> Iterator[bytes]:
    for i in INTS:
        yield i.to_bytes(length=4, byteorder="little")


def known_integer_packed_be_mutation(sample_input: bytes) -> Iterator[bytes]:
    for i in INTS:
        yield i.to_bytes(length=4, byteorder="big")


def known_integer_ascii_dec_mutation(sample_input: bytes) -> Iterator[bytes]:
    for i in INTS:
        yield f"{i}".encode()


def known_integer_ascii_hex_mutation(sample_input: bytes) -> Iterator[bytes]:
    for i in INTS:
        yield f"{i:x}".encode()


def known_integer_ascii_hex_with_prefix_mutation(
    sample_input: bytes,
) -> Iterator[bytes]:
    for i in INTS:
        yield f"0x{i:x}".encode()
