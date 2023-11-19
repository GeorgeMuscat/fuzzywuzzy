all: hang safe segv harness

harness: src/fuzzer/harness/harness.c src/fuzzer/harness/harness.h src/fuzzer/harness/hooks.c src/fuzzer/harness/hooks.h src/fuzzer/harness/socket.c src/fuzzer/harness/socket.h
	gcc -O0 -g -m32 -masm="intel" -fPIC -shared -Wl,--no-undefined -rdynamic -ldl src/fuzzer/harness/harness.c src/fuzzer/harness/hooks.c src/fuzzer/harness/socket.c -o src/fuzzer/harness/harness.so

hang: tests/binaries/hang/hang.c
	gcc -z execstack -fno-pie -fno-stack-protector -z norelro -m32 -g -o tests/binaries/hang/hang tests/binaries/hang/hang.c

safe: tests/binaries/safe/safe.c
	gcc -z execstack -fno-pie -fno-stack-protector -z norelro -m32 -g -o tests/binaries/safe/safe tests/binaries/safe/safe.c

segv: tests/binaries/segv/segv.c
	gcc -z execstack -fno-pie -fno-stack-protector -z norelro -m32 -g -o tests/binaries/segv/segv tests/binaries/segv/segv.c