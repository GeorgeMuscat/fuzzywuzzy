from pathlib import Path


from makeelf.elf import *

elf, _ = ELF.from_file(Path("tests", "binaries", "fuzz_targets", "plaintext2"))

wrapper, _ = ELF.from_file(Path("src", "fuzzer", "harness", "harness"))




for section in elf.Elf.Shdr_table:
    print(hex(section.sh_addr))

data_id = elf.append_section(".patched", b"\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", elf.Elf.Shdr_table[0].sh_addr + elf.Elf.Shdr_table[0].sh_size)
elf.append_segment(data_id, flags="rx")
# elf.asm(elf.plt["exit"], f"jmpd {hex(elf.symbols["main"])}")

#with open("tmp", "wb") as f:
    #f.write(bytes(elf))