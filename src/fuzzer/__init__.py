from pathlib import Path
from typing import Optional, Type

import click

from .formats import BaseFormat
from .mutations import BaseMutation


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
    print("Hello world!")
    print(sample_input, binary, output_file)


def fuzz(
    binary: Path, mutations: list[Type[BaseMutation]], formats: list[Type[BaseFormat]]
) -> Optional[bytes]:
    """do the thing. idk if these params make sense and maybe we want a class for this idk."""
    pass


def sanity():
    """Returns a constant for use in a test to ensure Python is not being weird about imports."""
    return 1337


if __name__ == "__main__":
    cli()
