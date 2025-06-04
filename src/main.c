#include "../include/mtgrep.h"

int main(int argc, char *argv[]) {
    search_t args = parse_input_args(argc, argv);
    find_patterns_parallel(args, NUMBER_OF_THREADS);
}