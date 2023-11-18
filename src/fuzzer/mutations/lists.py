from typing import Iterator


def add_to_list_mutator(list) -> Iterator[int]:
    for i in range(0, 20):
        list.append(list[-1])
        yield list


def long_items_list_mutator(list) -> Iterator[int]:
    for i in range(0, 10):
        list.append(list[-1] + list[-1])
        yield list