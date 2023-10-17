from pathlib import Path
from subprocess import DEVNULL, PIPE, Popen, TimeoutExpired

TIMEOUT = 5


class Harness:
    def __init__(self, binary_path: Path):
        self.binary_path = binary_path

    def run(self, input: bytes):
        process = Popen(self.binary_path, stdin=PIPE, stdout=DEVNULL, stderr=DEVNULL)

        try:
            process.communicate(input, timeout=TIMEOUT)
        except TimeoutExpired:
            process.terminate()
            return "timeout"

        return process.returncode
