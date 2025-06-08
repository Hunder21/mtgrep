#include "includes.h"

#define MIN_RESIZE 10

typedef struct {
    char * pattern;
    int number_of_names;
    char ** names;
} path_box; // PB

char ** get_dir_content (char * dir);

path_box pb_init (int argc, char *argv[]);

void pb_destroy (path_box pb);