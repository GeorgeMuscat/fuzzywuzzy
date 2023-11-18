import time
from pathlib import Path

from fuzzer import fuzz
from fuzzer.harness import TIMEOUT as HARNESS_TIMEOUT
from fuzzer.harness import Harness


MINUTE = 60  # certified Ham moment
FUZZING_TIMEOUT = MINUTE * 3


def test_fuzz(binary_path: tuple[Path, Path]):
    """Tests that we find a bad input for all test binaries, and that the result does crash the binary."""

    binary, input = binary_path
    with open(input, "rb") as f:
        start = time.time()
        output = fuzz(binary, f)
        assert output is not None, "could not find bad input"
        mutation, result, coverage = output
        end = time.time()
    assert result is not None
    assert mutation is not None

    harness = Harness(binary)
    return_code = harness.run(mutation)
    assert type(result["exit_code"]) is int
    assert result["exit_code"] < 0

    assert (end - start) < FUZZING_TIMEOUT


def test_hang_timeout():
    """Tests that the harness stops processes that last beyond the timeout."""
    hang_path = Path("tests", "binaries", "hang", "hang")

    harness = Harness(hang_path)

    start = time.time()
    assert harness.run(b"") == "timeout"
    end = time.time()

    assert (end - start) > HARNESS_TIMEOUT
