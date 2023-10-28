from typing import Iterator


def repeat_segment(sample_input: bytes, delimiter: bytes = b"\n") -> Iterator[bytes]:
    """
    Splits the input based on the provided delimiter (b"\n" is default), then
    for each segment, duplicates it and inserts it directly after the original segment
    in the sequence.
    """
    original_split = sample_input.split(delimiter)
    for i in range(len(original_split)):
        copy = original_split.copy()
        copy.insert(i + 1, original_split[i])
        yield delimiter.join(copy)


def overwrite_segment(sample_input: bytes, delimiter: bytes = b"\n") -> Iterator[bytes]:
    """
    Splits the input based on the provided delimiter (b"\n" is default), then
    for each segment, duplicates it and overwrites the directly preceding segment.

    If there is no preceding segment, the duplicate segment is appended to the sequence.
    """
    original_split = sample_input.split(delimiter)
    for i in range(len(original_split)):
        copy = original_split.copy()
        try:
            copy[i + 1] = copy[i]
        except:
            copy.append(copy[i])
        yield delimiter.join(copy)


def overwrite_segment_comma(sample_input: bytes) -> Iterator[bytes]:
    return overwrite_segment(sample_input, b",")
