import time
from pathlib import Path

from .base import Harness
from .inprocess import InProcessHarness
from .popen import PopenHarness


def bench(harness: type[Harness]):
    h = harness(Path("tests/binaries/fuzz_targets/plaintext2"))
    start = time.time()
    for i in range(1000):
        input = f"{i:06}\n".encode()
        result = h.run(input)
    return time.time() - start


def main():
    print("1000 runs, in-process harness:", bench(InProcessHarness))
    print("1000 runs, popen-based harness:", bench(PopenHarness))
