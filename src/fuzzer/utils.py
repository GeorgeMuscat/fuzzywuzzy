from datetime import datetime
from typing import Iterator, TypeVar
from rich.panel import Panel
from rich.live import Live
from rich.status import Status
from rich.console import Group

from rich.progress import Progress, TimeElapsedColumn, TextColumn

T = TypeVar("T")


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
    def __init__(self, binary: str) -> None:
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
        self.live = Live(
            self.panel,
            refresh_per_second=30,
        )
        self.live.start()

    def print(self, message: str):
        self.result_progress.add_task(message)
        self.live.refresh()

    def inc_mutations(self):
        self.mutations += 1
        self.mutation_status.update(self.__get_mutation_str(self.mutations))
        self.speed_status.update(self.__get_speed())

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
