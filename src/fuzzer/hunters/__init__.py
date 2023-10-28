from typing import Callable, Iterator

from .plaintext import segment_hunter, whole_text_hunter
from .json import json_key_hunter, json_value_hunter

Hunter = Callable[[bytes], Iterator[bytes]]
MIME_TYPE_TO_HUNTERS: dict[str, list[Hunter]] = {
    "text/plain": [whole_text_hunter],
    "application/octet-stream": [whole_text_hunter],
    "text/csv": [whole_text_hunter],
    "application/json": [json_key_hunter, json_value_hunter]
}
