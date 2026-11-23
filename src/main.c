/* A single main() launches the entire modular C11 application. */
#include "cli.h"
int main(int argc, char **argv) {
    return port_cli(argc, argv);
}
