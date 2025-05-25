#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dirent.h"
#include "sys/stat.h"

#define MAX_ARG_WIDTH 256
#define MAX_FILES_NUM 256

// Will use 4 threads
#define NUMBER_OF_THREADS 4

typedef struct {
    char pattern [MAX_ARG_WIDTH];
    char number_of_names;    
    char names   [MAX_FILES_NUM][MAX_ARG_WIDTH];
} search_t;

// 1 for dir, 0 for file (assume, that each dir ends with /)
int get_format (char * file);
// parse input args into struct
search_t parse_input_args (int argc, char *argv[]);

void print_args(search_t args);

char ** get_dir_content (char * dir);

search_t extract_filenames (search_t args);

int find_pattern_in_file (char * pattern, char * abs_path);

void find_patterns_seq (search_t args);

void * open_file_thread (void * search_data);

void find_patterns_parallel (search_t args, const int number_of_threads);
