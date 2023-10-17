# fuzzywuzzy

## Setup
- Install Python 3.12:
    - MacOS: `brew install python@3.12`
    - Linux: *figure it out*
- Install 32-bit support, including for `libc` and `gcc` (for building test binaries):
```bash
sudo dpkg --add-architecture i386
sudo apt-get -y update
sudo apt-get install -y libc6:i386 gcc-multilib g++-multilib
```
- Install Poetry: `curl -sSL https://install.python-poetry.org | python3 -`
- Install packages: `poetry install`

## Poetry Commands
- Install: `poetry install`
- Run command in venv: `poetry run`
- Enter sub-shell (like `source venv/bin/activate`): `poetry shell`
- Add dependency (don't use `pip`): `poetry add <pip package name>`

## Running Tests
To run all tests, run `pytest` (inside a Poetry shell, `poetry run pytest` outside).
