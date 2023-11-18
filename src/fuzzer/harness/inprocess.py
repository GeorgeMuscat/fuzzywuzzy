import os
import socket
import tempfile
import threading
import time
from pathlib import Path
from subprocess import DEVNULL, PIPE, Popen
from typing import Optional, TypedDict

from .base import BaseHarness

TIMEOUT = 1

MSG_ACK = 0x01
MSG_TARGET_START = 0x02
MSG_TARGET_RESET = 0x03
MSG_LIBC_CALL = 0x04
MSG_INPUT_REQUIRED = 0x05
MSG_INPUT_RESPONSE = 0x06


class FuzzerMessage(TypedDict):
    msg_type: int
    data: dict[str, int]


class InProcessHarness(BaseHarness):
    process: Popen
    connection: socket.socket
    open: bool
    debug: bool

    def __init__(self, binary_path: Path, debug: bool = False):
        self.binary_path = binary_path
        self.open = False
        self.debug = debug
        self.start()

    def run(self, input: bytes):
        if not self.open:
            self.start()

        assert self.process.stdin is not None

        self._await_start()
        start = time.time()
        self.process.stdin.write(input)
        self.process.stdin.flush()
        self.process.stdin.close()
        self._send_ack()

        events = []

        while True:
            exit_code = self.process.poll()

            if exit_code is not None:
                self.open = False
                if exit_code >= 0:
                    raise HarnessException("fuck the harness crashed...")

                duration = time.time() - start

                return {
                    "duration": duration,
                    "exit_code": exit_code,
                    "events": events,
                }

            msg = self._read_message()
            print(msg)

            if msg is None:
                continue

            if msg["msg_type"] == MSG_TARGET_RESET:
                duration = time.time() - start

                self.process.stdin = open(f"/proc/{self.process.pid}/fd/0", "wb")
                self._send_ack()

                exit_code = msg["data"]["exit_code"]
                events.append(("exit", exit_code))

                return {
                    "duration": duration,
                    "exit_code": exit_code,
                    "events": events,
                }
            elif msg["msg_type"] == MSG_LIBC_CALL:
                self._send_ack()
                events.append(
                    (
                        "libc_call",
                        msg["data"]["func_name"],
                        msg["data"]["return_addr"],
                    )
                )
            elif msg["msg_type"] == MSG_INPUT_REQUIRED:
                # determine if input requirement is satisfied
                self._write_message(MSG_INPUT_RESPONSE, {"can_satisfy": int(True)})
            else:
                raise UnexpectedMessageTypeException(
                    f"received unexpected message type {msg['msg_type']} during target execution"
                )

    def set_debug(self, debug: bool):
        self.debug = debug

    def start(self):
        socket_path = (
            f"/tmp/fuzzywuzzy_{self.binary_path.name}_{threading.get_ident()}.socket"
        )
        if os.path.exists(socket_path):
            os.remove(socket_path)

        self.server = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.server.bind(socket_path)
        self.server.listen(1)

        self.process = Popen(
            self.binary_path.absolute(),
            stdin=PIPE,
            stdout=None if self.debug else DEVNULL,
            stderr=None if self.debug else DEVNULL,
            env={
                "LD_PRELOAD": "./src/fuzzer/harness/harness.so",
                "FUZZYWUZZY_SOCKET_PATH": socket_path,
            },
        )

        self.connection, _ = self.server.accept()
        self.connection.settimeout(0.2)

        self.open = True

    def kill(self):
        self.process.kill()
        self.connection.close()
        self.open = False

    def restart(self):
        self.kill()
        self.start()

    def _read_bytes(self, bytes: int):
        return self.connection.recv(bytes)

    def _read_sized_int(self, bytes: int):
        b = self.connection.recv(bytes)
        if len(b) != bytes:
            return None
        return int.from_bytes(b, byteorder="little")

    def _read_int(self):
        return self._read_sized_int(4)

    def _read_size_t(self):
        return self._read_sized_int(4)

    def _read_uint8_t(self):
        return self._read_sized_int(1)

    def _write_sized_int(self, v: int, bytes: int):
        self.connection.send(v.to_bytes(bytes, byteorder="little"))

    def _write_uint8_t(self, v: int):
        self._write_sized_int(v, 1)

    def _write_bool(self, v: int):
        self._write_sized_int(v, 1)

    def _read_message(self) -> Optional[FuzzerMessage]:
        try:
            msg_type = self._read_uint8_t()
        except socket.timeout:
            msg_type = None
        data = {}

        if msg_type is None:
            return None

        if msg_type in [MSG_ACK, MSG_INPUT_RESPONSE]:
            raise UnexpectedMessageTypeException(
                f"did not expect to receive message type {msg_type}"
            )
        elif msg_type in [MSG_TARGET_START]:
            pass
        elif msg_type == MSG_TARGET_RESET:
            data["exit_code"] = self._read_int()
        elif msg_type == MSG_LIBC_CALL:
            data["func_name"] = self._read_bytes(64).rstrip(b"\0")
            data["return_addr"] = self._read_size_t()
        elif msg_type == MSG_INPUT_REQUIRED:
            data["flags"] = self._read_uint8_t()
            data["len"] = self._read_size_t()
        else:
            raise UnknownMessageTypeException(
                f"received unexpected message type {msg_type}"
            )

        if self.debug:
            print("received:", {"msg_type": msg_type, "data": data})
        return {"msg_type": msg_type, "data": data}

    def _write_message(self, msg_type: int, data: dict[str, int] = {}):
        if msg_type in [
            MSG_TARGET_START,
            MSG_TARGET_RESET,
            MSG_LIBC_CALL,
            MSG_INPUT_REQUIRED,
        ]:
            raise UnexpectedMessageTypeException(
                f"did not expect to send message type {msg_type}"
            )
        elif msg_type in [MSG_ACK, MSG_INPUT_RESPONSE]:
            self._write_uint8_t(msg_type)
            if self.debug:
                print("sent:", {"msg_type": msg_type, "data": data})
            if msg_type == MSG_ACK:
                pass
            elif msg_type == MSG_INPUT_RESPONSE:
                self._write_bool(data["can_satisfy"])
        else:
            raise UnknownMessageTypeException(
                f"tried to send unexpected message type {msg_type}"
            )

    def _await_start(self):
        msg = self._read_message()
        assert msg is not None
        assert (
            msg["msg_type"] == MSG_TARGET_START
        ), f"received msg type {hex(msg['msg_type'])}, was expecting {hex(MSG_TARGET_START)}"

    def _send_ack(self):
        self._write_message(MSG_ACK)


class HarnessException(Exception):
    pass


class UnexpectedMessageTypeException(HarnessException):
    pass


class UnknownMessageTypeException(HarnessException):
    pass


def main():
    harness = InProcessHarness(Path("tests/binaries/fuzz_targets/plaintext2"))
    print("result 1:", harness.run(b"trivial\n2\n"))
    print("result 2:", harness.run(b"trivial\n-524288\n"))
    print("result 3:", harness.run(b"trivial\n"))
    for i in range(1000):
        harness.run(b"trivial\n2\n")
    harness.set_debug(True)
    print("result 4:", harness.run(b"trivial\n-524288\n"))
