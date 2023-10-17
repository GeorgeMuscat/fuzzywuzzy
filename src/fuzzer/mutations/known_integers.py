from typing import Iterator

# A list of 'known' integers that may have some significance to the current binary - common buffer sizes, option menus, etc.
KNOWN_INTS: list[int] = list(
    {
        *(i for i in range(-10, 11)),  # -10 to 10
        100,
        101,
        0x7FFFFFFF,  # INT_MAX
        0x80000000,  # INT_MIN, 2^32
        0xFFFFFFFF,  # UINT_MAX, -1
        *(2**i for i in range(1, 32)),  # 2^i
        *(-(2**i) for i in range(1, 32)),  # -(2^i)
        *(2**i + 1 for i in range(1, 32)),  # 2^i + 1
    }
)


def known_integer_packed_le_mutation(sample_input: bytes) -> Iterator[bytes]:
    for i in KNOWN_INTS:
        try:
            yield i.to_bytes(length=4, byteorder="little")
        except OverflowError:
            yield i.to_bytes(length=4, byteorder="little", signed=True)


def known_integer_packed_be_mutation(sample_input: bytes) -> Iterator[bytes]:
    for i in KNOWN_INTS:
        try:
            yield i.to_bytes(length=4, byteorder="big")
        except OverflowError:
            yield i.to_bytes(length=4, byteorder="big", signed=True)


def known_integer_ascii_dec_mutation(sample_input: bytes) -> Iterator[bytes]:
    for i in KNOWN_INTS:
        yield f"{i}".encode()


def known_integer_ascii_hex_mutation(sample_input: bytes) -> Iterator[bytes]:
    for i in KNOWN_INTS:
        yield f"{i:x}".encode()


def known_integer_ascii_hex_with_prefix_mutation(
    sample_input: bytes,
) -> Iterator[bytes]:
    for i in KNOWN_INTS:
        yield hex(i).encode()
