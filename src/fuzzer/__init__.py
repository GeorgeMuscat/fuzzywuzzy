import os
from pathlib import Path
import sys
from typing import BinaryIO, Optional

import click
import magic
import signal

from fuzzer.harness import Harness
from fuzzer.utils import round_robin, Reporter
from .hunters import MIME_TYPE_TO_HUNTERS

reporter = None


@click.command()
@click.argument(
    "binary",
    type=click.Path(exists=True, executable=True, dir_okay=False, path_type=Path),
)
@click.argument("sample_input", type=click.File("rb"))
@click.option(
    "--output-file",
    help="path to output file",
    type=click.File("wb"),
    default="bad.txt",
)
def cli(binary: Path, sample_input: BinaryIO, output_file: BinaryIO):
    """Fuzzes BINARY, using SAMPLE_INPUT as a starting point."""
    global reporter
    reporter = Reporter(binary)

    def signal_handler(sig, frame):
        """
        Need this so that rich doesn't break the terminal cursor
        """
        reporter.reset_console() if reporter != None else None
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)

    result = fuzz(binary, sample_input)
    if result is not None:
        reporter.print_crash_output(100.3, -11, [("idk", 1000)])
        output_file.write(result)
    else:
        reporter.print("We couldn't break the binary T_T")

    reporter.reset_console()


def fuzz(binary: Path, sample_input_file: BinaryIO) -> Optional[bytes]:
    sample_input = sample_input_file.read()
    mime_type = magic.from_buffer(sample_input, mime=True)
    hunters = MIME_TYPE_TO_HUNTERS.get(mime_type)

    if hunters is None:
        # TODO: Don't use a CLI-specific error in a library function.
        raise click.UsageError(
            "sample_input_file does not contain data of a compatible format"
        )

    harness = Harness(binary)
    for mutation in round_robin([hunter(sample_input) for hunter in hunters]):
        result = harness.run(mutation)

        reporter.inc_mutations() if reporter != None else None
        if result["exit_code"] is not None and result["exit_code"] < 0:
            return mutation


def sanity():
    """Returns a constant for use in a test to ensure Python is not being weird about imports."""
    return 1337


if __name__ == "__main__":
    print("You pressed Ctrl+C!")
    cli()
