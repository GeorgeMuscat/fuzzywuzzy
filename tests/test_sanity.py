from fuzzer import sanity


def test_sanity():
    """Test that Python hasn't changed its' import semantics without warning us."""
    assert sanity() == 1337
