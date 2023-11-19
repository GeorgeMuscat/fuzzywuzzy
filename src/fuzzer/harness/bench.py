import time
from pathlib import Path

from .base import BaseHarness
from .inprocess import InProcessHarness
from .popen import PopenHarness


def bench(harness: type[BaseHarness]):
    h = harness(Path("tests/binaries/fuzz_targets/plaintext1"))
    start = time.time()
    for i in range(100000):
        input = f"{i:06}\n".encode()
        result = h.run(input)
    return time.time() - start


def main():
    print("100000 runs, in-process harness:", bench(InProcessHarness))
    print("100000 runs, popen-based harness:", bench(PopenHarness))
