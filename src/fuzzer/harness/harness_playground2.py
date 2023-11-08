from pathlib import Path

import lief


elf = lief.parse(Path("tests", "binaries", "fuzz_targets", "plaintext2").absolute().as_posix())

wrapper = lief.parse(Path("src", "fuzzer", "harness", "harness").absolute().as_posix())


main_replace_symbol = wrapper.get_symbol("main_replace")



code_segment = wrapper.segment_from_virtual_address(main_replace_symbol.value)
segment_added = elf.add(code_segment)

new_address = segment_added.virtual_address + main_replace_symbol.value - code_segment.virtual_address
print(hex(new_address))

elf.write("patched")