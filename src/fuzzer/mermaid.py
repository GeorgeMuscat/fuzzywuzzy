from typing import Optional

from fuzzer.coverage import CoverageGraph
from fuzzer.harness.base import HarnessResult
from fuzzer.utils import lookup_signal

BAD_INPUT_STYLE = "fill:#ff0000"


def display_event(event: tuple):
    typ = event[0]
    if typ == "libc_call":
        _, func_name, ret_addr = event
        return f"{func_name}\\n{hex(ret_addr)}"
    elif typ == "exit":
        _, exit_code = event
        return f"Exit\\nwith code: {exit_code}"
    return f"Unrecognised event\\n{event}"


def event_id(event: tuple, depth: int):
    return "-".join(str(e) for e in event) + f"-{depth}"


def get_lines(graph: CoverageGraph, parent_id: str, depth: int):
    lines: list[str] = []
    for event, child in graph.items():
        id = event_id(event, depth)
        lines.append(f'{id}["{display_event(event)}"]')
        lines.append(f"{parent_id}-->{id}")
        lines.extend(get_lines(child, id, depth + 1))

    return lines


def generate_mermaid_graph(graph: CoverageGraph, result: Optional[HarnessResult] = None):
    lines = get_lines(graph, "prog_start", 0)

    if result is not None:
        event_count = len(result["events"])
        id = event_id(result["events"][event_count - 1], event_count - 1)
        if result["exit_code"] is not None:
            lines.append(f'prog_end["Program crashed\\n{lookup_signal(result['exit_code'])} ({result['exit_code'] * -1})"]')
            for d, event in enumerate(result["events"]):
                lines.append(f"style prog_start {BAD_INPUT_STYLE}")
                lines.append(f"style {event_id(event, d)} {BAD_INPUT_STYLE}")
                lines.append(f"style prog_end {BAD_INPUT_STYLE}")
        else:
            lines.append(f'prog_end["Program terminated\\ndue to timeout"]')
        lines.append(f"{id}-->prog_end")

    return 'flowchart TD\n    prog_start["Program Start"]\n' + "\n".join(
        "    " + m for m in lines
    )
