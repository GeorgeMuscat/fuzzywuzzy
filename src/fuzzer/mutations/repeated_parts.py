from typing import Iterator, List


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
        try:
            yield delimiter.join(copy)
        except StopIteration:
            return


def repeat_last_segment_mutation(
    sample_input: bytes, delimiter: bytes = b"\n"
) -> Iterator[bytes]:
    """
    Splits the input based on the provided delimiter (b"\n" is default), then
    for each segment, duplicates it and inserts it directly after the original segment
    in the sequence.
    """
    original_split: List[bytes] = sample_input.split(delimiter)
    while len(original_split) > 0 and len(original_split[-1]) == 0:
        original_split.pop()
    if len(original_split) == 0:
        return
    for _ in range(100):
        original_split.append(original_split[-1])
        yield delimiter.join(original_split)
