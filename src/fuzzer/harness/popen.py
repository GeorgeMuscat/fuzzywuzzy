import time
from pathlib import Path
from subprocess import DEVNULL, PIPE, Popen, TimeoutExpired

from .base import Harness

TIMEOUT = 1


class PopenHarness(Harness):
    def __init__(self, binary_path: Path):
        self.binary_path = binary_path

    def run(self, input: bytes):
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
            "exit_code": process.returncode if not timed_out else None,
            "events": [],
        }
