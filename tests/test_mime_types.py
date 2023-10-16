from pathlib import Path

import magic

from fuzzer.hunters import MIME_TYPE_TO_HUNTERS


def test_mime_type_coverage(binary_path: tuple[Path, Path]):
    """Tests that we have hunters for all the sample inputs we care about."""
    _, input = binary_path
    mime_type = magic.from_file(input, mime=True)
    assert mime_type in MIME_TYPE_TO_HUNTERS
