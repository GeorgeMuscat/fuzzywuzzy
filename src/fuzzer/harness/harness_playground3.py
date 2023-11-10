import os.path
import socketserver
from pathlib import Path
import socket
from subprocess import DEVNULL, PIPE, Popen, TimeoutExpired

TIMEOUT = 1


class Harness3:
    process: Popen
    server: socket.socket

    def __init__(self, binary_path: Path):
        self.binary_path = binary_path
        self.server = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        socket_path = f"/tmp/socket_{binary_path.name}.s"
        if os.path.exists(socket_path):
            os.remove(socket_path)
        self.server.bind(socket_path)
        self.process = Popen(
            self.binary_path.absolute(), args=[socket_path], stdin=PIPE, stdout=DEVNULL, stderr=DEVNULL,
            env={"LD_PRELOAD": "./harness.so"}
        )


    def run(self, input: bytes):
        self.process.communicate(input)


        return self.process.returncode
