#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
gcc -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude src/main.c src/core/*.c src/algorithms/*.c -o main
gcc -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude tests/test_main.c src/core/*.c src/algorithms/*.c -o port_tests
./port_tests
./main --demo
./main --network
./main --benchmark
