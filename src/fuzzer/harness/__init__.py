from .base import BaseHarness
from .inprocess import InProcessHarness
from .popen import PopenHarness
from .inprocess import TIMEOUT

TIMEOUT = TIMEOUT
Harness: type[BaseHarness] = InProcessHarness
