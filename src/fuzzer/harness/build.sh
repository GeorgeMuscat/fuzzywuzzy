#!/bin/bash
gcc -Os -m32 -masm="intel" -fPIC -shared -Wl,--no-undefined -rdynamic harness.c socket.c hooks.c -o harness.so -ldl
