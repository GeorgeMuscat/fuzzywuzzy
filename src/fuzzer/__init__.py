import os
from pathlib import Path
from typing import BinaryIO, Optional

import click
import magic

from fuzzer.harness import Harness
from fuzzer.utils import round_robin

from .hunters import MIME_TYPE_TO_HUNTERS


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
def cli(binary, sample_input, output_file):
    """Fuzzes BINARY, using SAMPLE_INPUT as a starting point."""
    result = fuzz(binary, sample_input)
    if result is not None:
        click.echo("HOLY SHIT WE DID IT")
        output_file.write(result)
        click.echo("Check your cwd for bad.txt")
    else:
        click.echo("We couldn't break the binary T_T")


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
        if result["exit_code"] < 0:
            return mutation


def sanity():
    """Returns a constant for use in a test to ensure Python is not being weird about imports."""
    return 1337


if __name__ == "__main__":
    cli()
