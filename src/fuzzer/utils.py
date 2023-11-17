from datetime import datetime
from math import e
import time
from typing import Iterator, TypeVar, BinaryIO
from pathlib import Path
from rich.panel import Panel
from rich.live import Live
from rich.status import Status
from rich.console import Group, Console

from rich.progress import Progress, TimeElapsedColumn, TextColumn

T = TypeVar("T")

SIGNALS = {
    1: "SIGHUP",
    2: "SIGINT",
    3: "SIGQUIT",
    4: "SIGILL",
    5: "SIGTRAP",
    6: "SIGABRT",
    6: "SIGIOT",
    7: "SIGBUS",
    8: "SIGFPE",
    9: "SIGKILL",
    10: "SIGUSR1",
    11: "SIGSEGV",
    12: "SIGUSR2",
    13: "SIGPIPE",
    14: "SIGALRM",
    15: "SIGTERM",
    16: "SIGSTKFLT",
    17: "SIGCHLD",
    18: "SIGCONT",
    19: "SIGSTOP",
    20: "SIGTSTP",
    21: "SIGTTIN",
    22: "SIGTTOU",
    23: "SIGURG",
    24: "SIGXCPU",
    25: "SIGXFSZ",
    26: "SIGVTALRM",
    27: "SIGPROF",
    28: "SIGWINCH",
    29: "SIGIO",
    30: "SIGPWR",
    31: "SIGSYS",
}


def round_robin(iterables: list[Iterator[T]]) -> Iterator[T]:
    # Create an infinite loop to keep iterating in a round-robin fashion
    while True:
        for it in iterables:
            try:
                yield next(it)
            except (StopIteration, RuntimeError):
                # If an iterator is exhausted, remove it from the list
                iterables = [i for i in iterables if i is not it]
                if not iterables:
                    # All iterators are exhausted, exit the loop
                    return


class Reporter:
    def __init__(self, binary: Path) -> None:
        self.console = Console()
        self.mutations = 0
        self.init_time = datetime.now()

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
        self.live = Live(self.panel, refresh_per_second=30, console=self.console)
        self.live.start()

    def print(self, message: str):
        self.result_progress.add_task(message)
        self.live.refresh()

    def print_crash_output(self, duration: float, exit_code: int, events: list[tuple]):
        fn = f"./event-{int(time.time())}.txt"
        msg = f"""[bold red]Target Binary Crash Detected[/bold red]
    - Binary crashed with signal {lookup_signal(exit_code)}
    - Bad input took {duration:.2f} seconds to run
    - Notable events: {events if len(events) < 10 else f"[bold]See {fn} for a list of events[/bold]"}"""  # TODO: actually make events look ok
        if len(events) < 10:
            fp = open(fn, "w+")
            fp.write(str(events))
            fp.close()
        self.print(msg)

    def inc_mutations(self):
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
            return f"Speed: {self.mutations / (datetime.now() - self.init_time).total_seconds():.2f} mutations/sec"
        except ZeroDivisionError:
            return f"0 mutations/sec"


def lookup_signal(exit_code: int):
    """ """
    return SIGNALS[-1 * exit_code]
