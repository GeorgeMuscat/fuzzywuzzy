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

    assert type(result["exit_code"]) is int
    assert result["exit_code"] < 0

    assert (end - start) < FUZZING_TIMEOUT
