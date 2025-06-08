#include "../include/mtgrep.h"

int main(int argc, char *argv[]) {
    // search_t args = parse_input_args(argc, argv);
    // find_patterns_parallel(args, NUMBER_OF_THREADS);
    path_box pb = pb_init(argc, argv);
    find_patterns_parallel(pb, NUMBER_OF_THREADS);
    pb_destroy(pb);
    return 0;

}