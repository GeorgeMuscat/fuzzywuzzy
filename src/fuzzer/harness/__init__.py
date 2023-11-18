from .base import BaseHarness
from .inprocess import InProcessHarness
from .popen import PopenHarness

Harness: type[BaseHarness] = InProcessHarness
