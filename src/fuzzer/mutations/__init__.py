from typing import Callable, Iterator

Mutation = Callable[[bytes], Iterator[bytes]]
