#!/bin/bash
gcc -O0 -m32 -masm="intel" -fPIC -shared -Wl,--no-undefined -rdynamic harness.c socket.c hooks.c -o harness.so -ldl
