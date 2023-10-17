from typing import Callable, Iterator

from .plaintext import segment_hunter, whole_text_hunter

Hunter = Callable[[bytes], Iterator[bytes]]
MIME_TYPE_TO_HUNTERS: dict[str, list[Hunter]] = {
    "text/plain": [whole_text_hunter, segment_hunter(b"\n")],
    "application/octet-stream": [whole_text_hunter, segment_hunter(b"\n")],
    "text/csv": [whole_text_hunter, segment_hunter(b"\n")],
}
