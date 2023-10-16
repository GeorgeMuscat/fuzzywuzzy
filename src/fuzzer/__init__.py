from pathlib import Path
from typing import Any, Optional, Type

import click
import magic

from .hunters import MIME_TYPE_TO_HUNTERS


@click.command()
@click.argument("sample_input", type=click.File("rb"))
@click.argument(
    "binary",
    type=click.Path(exists=True, executable=True, dir_okay=False, path_type=Path),
)
@click.option(
    "--output-file",
    help="path to output file",
    type=click.File("wb"),
    default="bad.txt",
)
def cli(sample_input, binary, output_file):
    """Fuzzes BINARY, using SAMPLE_INPUT as a starting point."""
    fuzz(binary, sample_input)


def fuzz(binary: Path, sample_input) -> Optional[bytes]:
    """do the thing. idk if these params make sense and maybe we want a class for this idk."""
    sample_input = sample_input.read()
    file_type = magic.from_buffer(sample_input, mime=True)
    hunters = MIME_TYPE_TO_HUNTERS.get(file_type)

    if hunters is None:
        raise click.UsageError("sample_input is not a compatible file type.")

    print("did thing", format)


def sanity():
    """Returns a constant for use in a test to ensure Python is not being weird about imports."""
    return 1337


if __name__ == "__main__":
    cli()
