from typing import Iterator, TypeVar

T = TypeVar("T")

SIGNALS = {
    1: "SIGHUP",
    2: "SIGINT",
    3: "SIGQUIT",
    4: "SIGILL",
    5: "SIGTRAP",
    6: "SIGABRT",
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


def round_robin(iterators: list[Iterator[T]]) -> Iterator[T]:
    """Get the next item from each iterator in a round robin."""
    l = len(iterators)
    idx = 0
    while True:
        it = iterators[idx]

        try:
            yield next(it)
            idx += 1
        except StopIteration:
            iterators.pop(idx)
            l -= 1
            if l == 0:
                return

        if idx >= l:
            idx = 0


T = TypeVar("T")
U = TypeVar("U")


def tag(tag: T, iterator: Iterator[U]) -> Iterator[tuple[T, U]]:
    """Tags each element from `iterator` with a fixed value `tag`. e.g. to identify it's source later."""
    while True:
        try:
            yield (tag, next(iterator))
        except StopIteration:
            return


def lookup_signal(exit_code: int):
    """Looks up the name of a signal based on an exit code from popen (should be negative)."""
    return SIGNALS[-1 * exit_code]
