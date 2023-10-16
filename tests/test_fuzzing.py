from pathlib import Path

from fuzzer import fuzz
from fuzzer.harness import Harness


def test_fuzz(binary_path: tuple[Path, Path]):
    """Tests that we find a bad input for all test binaries, and that the result does crash the binary."""

    binary, input = binary_path
    with open(input, "rb") as f:
        result = fuzz(binary, f)
    assert result is not None

    harness = Harness(binary)
    return_code = harness.run(result)
    assert type(return_code) is int
    assert return_code < 0
