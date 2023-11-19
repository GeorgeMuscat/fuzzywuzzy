import json
import time
from pathlib import Path

from rich.console import Console, Group
from rich.live import Live
from rich.panel import Panel
from rich.progress import Progress, TextColumn, TimeElapsedColumn
from rich.status import Status

from fuzzer.coverage import CoverageGraph
from fuzzer.harness.base import HarnessResult
from fuzzer.mermaid import generate_mermaid_graph
from fuzzer.utils import lookup_signal


class Reporter:
    def __init__(self, binary: Path) -> None:
        self.console = Console()
        self.mutations = 0
        self.init_time = time.time()

        self.mutation_status = Status(self.__get_mutation_str(self.mutations))
        self.time_progress = Progress("Time Elapsed:", TimeElapsedColumn())
        self.speed_status = Status(self.__get_speed())
        self.result_progress = Progress(TextColumn("{task.description}"))
        self.time_progress.add_task("Time Elapsed")
        self.panel = Panel(
            Group(
                f"[bold]Fuzzing [underline]{binary}",
                self.mutation_status,
                self.time_progress,
                self.speed_status,
                self.result_progress,
            )
        )
        self.live = Live(self.panel, refresh_per_second=5, console=self.console)
        self.live.start()

    def print(self, message: str):
        self.result_progress.add_task(message)
        self.live.refresh()

    def print_crash_output(self, result: HarnessResult, graph: CoverageGraph):
        assert result["exit_code"] is not None

        ts = int(time.time())
        events_fn = f"./events-{ts}.json"
        mermaid_fn = f"./coverage-{ts}.mermaid"

        msg = f"""[bold red]Target Binary Crash Detected[/bold red]
    - Binary crashed with signal {lookup_signal(result['exit_code'])}
    - Bad input took {result['duration']:.2f} seconds to run
    - Notable events: {result['events'] if len(result['events']) < 10 else f"[bold]See {events_fn} for a list of events[/bold]"}
    - Coverage graph: [bold]See {mermaid_fn} for a mermaid.js graph of coverage information (copy paste contents into https://mermaid.live/)[/bold]"""  # TODO: actually make events look ok

        if len(result["events"]) >= 10:
            with open(events_fn, "w") as f:
                f.write(json.dumps(result["events"], indent=4))

        with open(mermaid_fn, "w") as f:
            f.write(generate_mermaid_graph(graph, result))
        self.print(msg)

    def log_result(self, result: HarnessResult):
        self.mutations += 1
        self.mutation_status.update(self.__get_mutation_str(self.mutations))
        self.speed_status.update(self.__get_speed())

    def reset_console(self):
        self.console.show_cursor()

    def __get_mutation_str(self, mutations: int):
        return f"Mutations: {mutations}"

    def __get_speed(self) -> str:
        """
        Returns the speed in mutations/second rounded to 2 decimal places.
        """
        try:
            return f"Speed: {self.mutations / (time.time() - self.init_time):.2f} mutations/sec"
        except ZeroDivisionError:
            return f"0 mutations/sec"
