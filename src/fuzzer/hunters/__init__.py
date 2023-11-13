from typing import Callable, Iterator

from .plaintext import segment_hunter, whole_text_hunter
from .json import json_key_hunter, json_value_hunter
from .jpeg import marker_hunter, header_hunter, quantization_table_hunter

Hunter = Callable[[bytes], Iterator[bytes]]
MIME_TYPE_TO_HUNTERS: dict[str, list[Hunter]] = {
    "text/plain": [whole_text_hunter, segment_hunter(b"\n")],
    "application/octet-stream": [whole_text_hunter, segment_hunter(b"\n")],
    "text/csv": [whole_text_hunter],
    "application/json": [json_key_hunter, json_value_hunter],
    "text/html": [whole_text_hunter, segment_hunter(b"\n")],
    "image/jpeg": [marker_hunter, header_hunter, quantization_table_hunter],
}
