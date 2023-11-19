CoverageGraph = dict[tuple, "CoverageGraph"]


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
