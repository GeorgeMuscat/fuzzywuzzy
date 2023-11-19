from .base import BaseHarness
from .inprocess import InProcessHarness
from .popen import PopenHarness

# Default harness.
Harness: type[BaseHarness] = InProcessHarness
