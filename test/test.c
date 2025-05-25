#include "../include/mtgrep.h"
#include "time.h"

int main (int argc, char *argv[]) {
    if (argc < 3)
        return 1;
    search_t args = parse_input_args(argc, argv);

    clock_t timer_start;
    clock_t timer_end;
    clock_t time_eval;

    printf("Seq test!\n");
    timer_start = clock();
    find_patterns_seq(args);
    timer_end = clock();
    
    time_eval = timer_end - timer_start;
    printf("Evaluated time: %ld\n", time_eval);

    printf("Parallel test!\n");
    timer_start = clock();
    find_patterns_parallel(args, NUMBER_OF_THREADS);
    timer_end = clock();

    time_eval = timer_end - timer_start;
    printf("Evaluated time: %ld\n", time_eval);

    return 0;
}