#!/bin/bash
gcc -O0 -g -m64 -masm="intel" -fPIC -shared -Wl,--no-undefined -rdynamic harness.c socket.c hooks.c -o harness64.so -ldl
