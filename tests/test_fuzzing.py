import time
from io import BytesIO
from pathlib import Path

import pytest

from fuzzer import fuzz
from fuzzer.harness import Harness

MINUTE = 60  # certified Ham moment
FUZZING_TIMEOUT = MINUTE * 3


def test_fuzz(binary_path: tuple[Path, Path]):
    """Tests that we find a bad input for all test binaries, and that the result does crash the binary."""

    binary, input = binary_path
    with open(input, "rb") as f:
        start = time.time()
        result = fuzz(binary, f)
        end = time.time()
    assert result is not None, "could not find bad input"

    harness = Harness(binary)
    result = harness.run(result[0])
    assert result["exit_code"] is not None and result["exit_code"] < 0

    assert (end - start) < FUZZING_TIMEOUT


@pytest.mark.skip(reason="takes ages to try every mutation")
def test_safe():
    """Tests that we CAN'T find a bad input for a binary that has no vulnerabilities."""
    with BytesIO(b"") as f:
        result = fuzz(Path("tests", "binaries", "safe", "safe"), f)
    assert result is None, "somehow found bad input"


def test_hang_timeout():
    """Tests that the harness stops processes that last beyond the timeout."""
    harness = Harness(Path("tests", "binaries", "hang", "hang"))

    start = time.time()
    assert harness.run(b"")["exit_code"] is None
    assert (time.time() - start) > Harness.TIMEOUT


def test_detect_segfault():
    """Tests that the harness can detect a segfault correctly."""
    harness = Harness(Path("tests", "binaries", "segv", "segv"))

    start = time.time()
    assert harness.run(b"")["exit_code"] == -11
    assert (time.time() - start) > Harness.TIMEOUT
