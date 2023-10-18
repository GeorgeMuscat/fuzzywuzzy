from typing import Iterator


def repeat_segment(sample_input: bytes, delimiter: bytes = b"\n") -> Iterator[bytes]:
    """
    Splits the input based on the provided delimiter (b"\n" is default), then
    for each segment, duplicates it and inserts it directly after the original segment
    in the sequence.
    """
    original_split = sample_input.split(delimiter)
    for i in range(original_split):
        copy = original_split.copy()
        copy.insert(i + 1, original_split[i])
        yield delimiter.join(copy)
