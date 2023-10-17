from os import listdir
from pathlib import Path

import pytest

BINARIES_DIR = Path("tests", "binaries", "fuzz_targets")


def get_binary_test_cases():
    return list(set([fn.removesuffix(".txt") for fn in listdir(BINARIES_DIR)]))


def pytest_generate_tests(metafunc: pytest.Metafunc):
    if "binary_path" in metafunc.fixturenames:
        metafunc.parametrize("binary_path", get_binary_test_cases(), indirect=True)


@pytest.fixture()
def binary_path(request):
    return (BINARIES_DIR / request.param, BINARIES_DIR / f"{request.param}.txt")
