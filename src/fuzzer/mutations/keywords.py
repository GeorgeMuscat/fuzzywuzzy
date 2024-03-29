from typing import Iterator


def delete_keywords(sample_input: bytes, keywords: list[bytes]) -> Iterator[bytes]:
    for keyword in keywords:
        yield sample_input[:].replace(keyword, b"")


def repeat_keywords(sample_input: bytes, keywords: list[bytes]) -> Iterator[bytes]:
    for keyword in keywords:
        for n in range(32):
            yield sample_input[:].replace(keyword, keyword * n)
