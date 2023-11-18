from typing import Generic, Iterator, TypeVar

T = TypeVar("T")


class WeightedRoundRobinIterator(Generic[T]):
    def __init__(self, initial_items: list[tuple[int, T]]):
        self.items = [(weight, item) for weight, item in initial_items if weight > 0]
        # Index of the current item (the item last returned).
        self.idx = 0
        # Number of times the current item has been returned.
        self.progress = 0

    def __iter__(self):
        return self

    def set_current_weight(self, weight: int):
        _, item = self.items[self.idx]
        if weight <= 0:
            self.items.pop(self.idx)
            self.progress = 0
        else:
            self.items[self.idx] = (weight, item)

    def get_current(self):
        return self.items[self.idx]

    def append_next(self, weight: int, item: T):
        """Adds an item into the next position in the queue."""
        if weight <= 0:
            return
        self.items.insert(self.idx + 1, (weight, item))

    def __next__(self):
        # Get the current item and it's weight.
        weight, item = self.items[self.idx]

        # If we have already returned this item that many times...
        if self.progress == weight:
            # Reset to zero progress.
            self.progress = 0

            # Move to the next item and check again.
            self.idx += 1
            if self.idx >= len(self.items):
                self.idx = 0

            _, item = self.items[self.idx]

        self.progress += 1
        return item


class WeightedRoundRobinFlatteningIterator(
    Generic[T], WeightedRoundRobinIterator[Iterator[T]]
):
    """Wrapper around WRRI to support flattening and exhausting iterator items."""

    def __init__(self, initial_items: list[tuple[int, Iterator[T]]]):
        super().__init__(initial_items)

    def __next__(self):
        iter = super().__next__()
        try:
            return next(iter)
        except StopIteration as e:
            self.set_current_weight(0)
            raise e
