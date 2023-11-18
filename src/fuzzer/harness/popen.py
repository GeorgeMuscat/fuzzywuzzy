import time
from pathlib import Path
from subprocess import DEVNULL, PIPE, Popen, TimeoutExpired

from .base import BaseHarness, HarnessResult

TIMEOUT = 1


class PopenHarness(BaseHarness):
    def __init__(self, binary_path: Path, debug: bool = False):
        self.binary_path = binary_path

    def run(self, input: bytes) -> HarnessResult:
        start = time.time()

        process = Popen(
            self.binary_path.absolute(), stdin=PIPE, stdout=DEVNULL, stderr=DEVNULL
        )

        timed_out = False

        try:
            process.communicate(input, timeout=TIMEOUT)
        except TimeoutExpired:
            process.terminate()
            timed_out = True
        duration = time.time() - start

        return {
            "duration": duration,
            "exit_code": None if timed_out else process.returncode,
            "events": [],
        }
