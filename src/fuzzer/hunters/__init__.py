from typing import Callable, Iterator

from .plaintext import line_hunter, whole_text_hunter

Hunter = Callable[[bytes], Iterator[bytes]]
MIME_TYPE_TO_HUNTERS: dict[str, list[Hunter]] = {
    "text/plain": [whole_text_hunter, line_hunter],
    "application/octet-stream": [whole_text_hunter, line_hunter],
}
