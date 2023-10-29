# FuzzyWuzzy

###### Etymology
george - enough said

## Installation

In the project root (requires sudo):
```bash
./install.sh
```

If the project gives a `command not found` error, check that the `pip` script directory (typically `~/.local/bin`) is in your `PATH`.

## Usage
In the project root:
```bash
./fuzzer <binary> <sample_input>
```

Alternatively, in any directory on your system:
```
fuzzywuzzy <binary> <sample_input>
```

## Current Functionality
* Finds inputs to crash plaintext[123], json1 and csv[12].
* Outputs the first crashing input to a file `bad.txt`
* Has basic detection of the process hanging/waiting for input.

## Design

* Modular
    * FuzzyWuzzy is designed to allow new fuzzing strategies to be implemented with extremely minimal changes to existing code. Strategies are formed through a combination of `Hunters` and `Mutators`.
* Hunters
    * Hunters, defined for each file input type, find bytes of interest in the sample input which can be mutated to potentially produce an input that might crash the program.
    * These bytes are passed onto mutators (see below) that return mutated (or replacement) bytes. The hunter then serialises the mutated bytes back into the original input.
        * This allows for a mutator to include, for example, backslashes in a JSON string, or a length field in a binary format to be updated based on the modification made to the field it descibes - of course, some hunters may choose to skip these steps to see what breaks.
    * Targeting specific bytes for mutation allows us to continue to provide valid inputs to programs which expect inputs to adhere to certain file formats, such that the target binary doesn't early-exit.
    * Each hunter then also defines which mutatators can change the selected bytes.
* Mutators
    * Mutators receive a specific byte input from a hunter, which they change in some predefined way, depending on the mutator.
    * Some example mutators flip random bits, repeat segments (e.g. lines), attempt buffer overflows, replace numbers with known integers, etc.
    * Mutator inputs are run in a round robin fashion, such that we try as wide a range of strategies as possible, when the target may not necessarily be vulnerable to all the bug types a certain mutator targets.

## Further Goals
* In-memory resetting
    * FuzzyWuzzy should not have to reload a binary from disk in order to provide a new bad input.
    * To do this, we plan to modify the binary on first load before initial execution, and replacing the `exit` function and adding additional shellcode to the end of `_start`.
* Weighted round robin
    * FuzzyWuzzy should be able to prioritise certain strategies depending on the strategies general effectiveness.
* Coverage
    * FuzzyWuzzy should measure which code paths past mutated inputs have taken.
* Coverage based mutation
    * FuzzyWuzzy should favour mutations which increase the number of code paths taken, as this increases the likelihood of finding a bug.
* CLI
    * FuzzyWuzzy should be intuitive to interact with and provide a helpful CLI.