CC ?= gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic -Iinclude
CORE = $(wildcard src/core/*.c)
ALGS = $(wildcard src/algorithms/*.c)
all: main
main: src/main.c $(CORE) $(ALGS) include/port.h include/algorithms.h include/network.h include/cli.h
	$(CC) $(CFLAGS) src/main.c $(CORE) $(ALGS) -o main
port_tests: tests/test_main.c $(CORE) $(ALGS)
	$(CC) $(CFLAGS) tests/test_main.c $(CORE) $(ALGS) -o port_tests
test: port_tests
	./port_tests
demo: main
	./main --demo
benchmark: main
	./main --benchmark
clean:
	rm -f main port_tests
.PHONY: all test demo benchmark clean
