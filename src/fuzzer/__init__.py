import signal
import sys
from pathlib import Path
from typing import BinaryIO, Callable, Optional

import click
import magic

from fuzzer.harness import Harness, PopenHarness
from fuzzer.harness.base import HarnessResult
from fuzzer.utils import Reporter, round_robin
from fuzzer.wrr import WeightedRoundRobinFlatteningIterator

from .hunters import MIME_TYPE_TO_HUNTERS, Hunter

CoverageGraph = dict[tuple, "CoverageGraph"]


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
    reporter = Reporter(binary)

    def signal_handler(sig, frame):
        """
        Need this so that rich doesn't break the terminal cursor
        """
        reporter.reset_console()
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)

    result = fuzz(binary, sample_input, reporter.log_result)
    if result is not None:
        reporter.print_crash_output(result[1])
        output_file.write(result[0])
    else:
        reporter.print("We couldn't break the binary T_T")

    reporter.reset_console()


def mutation_iterator(hunters: list[Hunter], sample_input: bytes):
    return round_robin([hunter(sample_input) for hunter in hunters])


def fuzz(
    binary: Path,
    sample_input_file: BinaryIO,
    result_callback: Callable[[HarnessResult], None] = lambda r: None,
) -> Optional[tuple[bytes, HarnessResult, CoverageGraph]]:
    sample_input = sample_input_file.read()
    mime_type = magic.from_buffer(sample_input, mime=True)
    hunters = MIME_TYPE_TO_HUNTERS.get(mime_type)

    if hunters is None:
        hunters = MIME_TYPE_TO_HUNTERS["text/plain"]
        # TODO: Don't use a CLI-specific error in a library function.
        # raise click.UsageError(
        #     "sample_input_file does not contain data of a compatible format"
        # )

    if "64-bit" in magic.from_file(binary):
        harness = PopenHarness(binary)
    else:
        harness = Harness(binary)

    coverage_graph = {}

    # Initialise the graph with the coverage of the sample input.
    result = harness.run(sample_input)
    initial_nodes = merge_coverage_events(coverage_graph, result["events"])

    wrr_iter = WeightedRoundRobinFlatteningIterator(
        [(initial_nodes, mutation_iterator(hunters, sample_input))]
    )

    for mutation in wrr_iter:
        result = harness.run(mutation)
        new_nodes = merge_coverage_events(coverage_graph, result["events"])

        if new_nodes > 0:
            print(new_nodes)
            # If we found new branches (calls to libc), then mutate this input further.
            # More nodes uncovered = more code and more opportunities for bugs, so give it more weight.
            wrr_iter.append_next(new_nodes, mutation_iterator(hunters, mutation))

        result_callback(result)
        if result["exit_code"] is not None and result["exit_code"] < 0:
            return mutation, result, coverage_graph

    return None


def merge_coverage_events(graph: CoverageGraph, events: list[tuple]):
    """Adds `events` into `graph` and returns the number of newly discovered nodes."""
    head = graph
    new_nodes = 0
    for event in events:
        if event not in head:
            head[event] = {}
            new_nodes += 1
        head = head[event]
    return new_nodes


def sanity():
    """Returns a constant for use in a test to ensure Python is not being weird about imports."""
    return 1337


if __name__ == "__main__":
    print("You pressed Ctrl+C!")
    cli()
