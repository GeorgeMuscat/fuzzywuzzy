from .example import hello_world


def cli():
    hello_world()


def sanity():
    """Returns a constant for use in a test to ensure Python is not being weird about imports."""
    return 1337


if __name__ == "__main__":
    cli()
