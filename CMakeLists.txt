cmake_minimum_required(VERSION 3.13)
project(ContainerPortDAA C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(PORT_CORE
    src/core/io.c
    src/core/scheduler.c
    src/core/report.c
    src/core/fleet.c
    src/core/network.c
    src/core/benchmark.c
    src/core/cli.c)
set(PORT_ALGORITHMS
    src/algorithms/merge_sort.c
    src/algorithms/binary_search.c
    src/algorithms/interval_partition.c
    src/algorithms/knapsack.c
    src/algorithms/dijkstra.c
    src/algorithms/bellman_ford.c
    src/algorithms/kruskal.c
    src/algorithms/max_flow.c)
add_executable(main src/main.c ${PORT_CORE} ${PORT_ALGORITHMS})
target_include_directories(main PRIVATE include)
add_executable(port_tests tests/test_main.c ${PORT_CORE} ${PORT_ALGORITHMS})
target_include_directories(port_tests PRIVATE include)
if(MSVC)
    target_compile_options(main PRIVATE /W4)
    target_compile_options(port_tests PRIVATE /W4)
else()
    target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(port_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
enable_testing()
add_test(NAME all_port_tests COMMAND port_tests)
set_tests_properties(all_port_tests PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
