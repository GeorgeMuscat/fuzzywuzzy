from fuzzer import sanity


def test_sanity():
    assert sanity() == 1337
