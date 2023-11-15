from typing import Callable, Iterator

from .plaintext import segment_hunter, whole_text_hunter
from .jpeg import marker_hunter, header_hunter, quantization_table_hunter
from .json import json_key_hunter, json_value_hunter, json_key_remover
from .xml import (
    xml_attribute_hunter,
    xml_text_hunter,
    xml_tag_hunter,
    xml_repeated_lines,
)

Hunter = Callable[[bytes], Iterator[bytes]]
MIME_TYPE_TO_HUNTERS: dict[str, list[Hunter]] = {
    "text/plain": [whole_text_hunter, segment_hunter(b"\n")],
    "application/octet-stream": [whole_text_hunter, segment_hunter(b"\n")],
    "text/csv": [whole_text_hunter],
    "image/jpeg": [marker_hunter, header_hunter, quantization_table_hunter],
    "application/json": [
        json_key_hunter,
        json_value_hunter,
        json_key_remover,
        whole_text_hunter,
    ],
    "text/html": [
        xml_attribute_hunter,
        xml_text_hunter,
        xml_tag_hunter,
        xml_repeated_lines,
        whole_text_hunter,
        segment_hunter(b"\n"),
    ],
}
