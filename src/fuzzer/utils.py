from typing import Iterator, TypeVar

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
