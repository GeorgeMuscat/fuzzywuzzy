#!/bin/bash
gcc -Os -m32 -masm="intel" -fPIC -Wl,-shared -static-libgcc -s harness.c -o harness
