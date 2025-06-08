#include "../include/mtgrep.h"

search_t extract_filenames (search_t args) {
    // resulting structure
    search_t args_rev = {0};
    // copy pattern
    snprintf(args_rev.pattern, MAX_ARG_WIDTH, "%s", args.pattern);
    // main loop through each name in args
    for (int i = 0; i < args.number_of_names; ++i) {
        // extract new name:
        char * name = args.names[i];
        
        // get name status
        struct stat name_stat;

        if (stat(name, &name_stat) == -1) {
            fprintf(stderr, "Couldn't get stat: %s!\n", name);
            exit(1);
        }

        // process status
        if (S_ISREG(name_stat.st_mode)) {
            // Regular file
            snprintf(args_rev.names[(size_t)args_rev.number_of_names], MAX_ARG_WIDTH, "%s", name);
            args_rev.number_of_names++;
        } else if (S_ISDIR(name_stat.st_mode)) {
            // Directory
            char ** dir_content = get_dir_content(name);
            char * entry;
            int i = 0;
            while((entry = *(dir_content + i)) != NULL) {
                char full_path [MAX_ARG_WIDTH];
                struct stat entry_stat;

                snprintf(full_path, MAX_ARG_WIDTH, "%s%s", name, entry);
                
                if(stat(full_path, &entry_stat) == -1) {
                    fprintf(stderr, "Couldn't get stat: %s!\n", full_path);
                    exit(1);
                }

                if(S_ISREG(entry_stat.st_mode)) {
                    snprintf(args_rev.names[(size_t)args_rev.number_of_names], MAX_ARG_WIDTH, "%s", full_path);
                    args_rev.number_of_names++;
                }

                i++;
            }   

            free(dir_content);
        }
    }
    return args_rev;
}

search_t parse_input_args(int argc, char *argv[]) {
    search_t args = {0};
    snprintf(args.pattern, MAX_ARG_WIDTH, "%s", argv[1]);
    for(int i = 2; i < argc; ++i) {
        snprintf(args.names[(size_t)args.number_of_names], MAX_ARG_WIDTH, "%s", argv[i]);
        args.number_of_names ++;
    }
    return extract_filenames(args);
}

void print_args(search_t args) {
    printf("Pattern: \"%s\"\n", args.pattern);
    printf("names: ");
    for(int i = 0; i < args.number_of_names; ++i) {
        printf("%s ", args.names[i]);
    }
    printf("\n");
}

int find_symb (char symb, int pos, char symb_arr []) {
    for(int i = pos - 1; i >= 0; --i) {
        if (symb_arr[i] == symb)
        return i;
    }
    return -1;
}

// Speeded up Boyer Moore bad symbol pattern search algorithm.

int find_pattern_in_file_Boyer_Moore (char * pattern, char * abs_path) {
    const int pattern_len = strlen(pattern);
    
    ////////////////
    FILE * fd = fopen(abs_path, "r");

    // use IO file bufferization
    // to reduce number of sys calls.
    char io_buf[8192];
    setvbuf(fd, io_buf, _IOFBF, sizeof(io_buf));
    
    char sliding_window [pattern_len+1];

    if(fd == NULL) {
        fprintf(stderr, "Warning: Couldn't open file: %s!\n", abs_path);
        return 0;
    }

    // read sliding window initial state
    if(fgets(sliding_window, pattern_len+1, fd) == NULL) {
        return 0;
    }
    sliding_window[pattern_len] = '\0';

    int j = pattern_len - 1;
    while(1) {
        if(pattern[j] == sliding_window[j]) {
            if(j == 0) {
                fclose(fd);
                return 1;
            }
            j--;
            continue;
        } else {
            j = pattern_len - 1;
            int pos_to_left;
            if((pos_to_left = find_symb(sliding_window[j], j, pattern)) == -1) {
                // shift by pattern len
                if(fgets(sliding_window, pattern_len+1, fd) == NULL) {
                    fclose(fd);
                    return 0;
                }
            } else {
                // shift by pos_to_left
                for(int i = 0; i < pattern_len - pos_to_left - 1; ++i) {
                    for(int k = 0; k < pattern_len-1; ++k) {
                        sliding_window[k] = sliding_window[k+1];
                    }
                    char next_ch;
                    if ((next_ch = fgetc(fd)) != EOF) {
                        sliding_window[pattern_len-1] = next_ch;
                    } else {
                        fclose(fd);
                        return 0;
                    }
                }
            }
        }
    }
}

// Simple find pattern algorithm

int find_pattern_in_file (char * pattern, char * abs_path) {
    FILE *fd = fopen(abs_path, "r");
    
    if (fd == NULL) {
        fprintf(stderr, "Couldn't open file: %s!\n", abs_path);
        return 0;
    }

    const size_t pattern_size = strlen(pattern);
    char sliding_window [pattern_size+1];
    sliding_window[pattern_size] = '\0';

    if(fgets(sliding_window, pattern_size, fd) == NULL) {
        return 0;
    }
    sliding_window[pattern_size] = '\0';
    
    while(1) {
        char next_ch = fgetc(fd);
        if (strcmp(pattern, sliding_window) == 0) {
            fclose(fd);
            return 1;
        }
        if(next_ch != EOF) {
            for(size_t i = 0; i < pattern_size-1; ++i) {
                sliding_window[i] = sliding_window[i+1];
            }
            sliding_window[pattern_size-1] = next_ch;
        } else break;
    }
    fclose(fd);
    return 0;
}

void find_patterns_seq (search_t args) {
    size_t meets = 0;
    printf("Pattern: \"%s\"\n", args.pattern);
    for(int i = 0; i < args.number_of_names; ++i) {
        if(find_pattern_in_file_Boyer_Moore(args.pattern, args.names[i]) == 1)
        {   
            meets++;
            printf("Met pattern in \"%s\"!\n", args.names[i]);
        }
    }
    printf("Meets: %lu\n", meets);
}

void * open_file_thread (void * sdata) {

    char ** data = (char **)sdata;

    int * ret_val = calloc(1, sizeof(int));
    if(ret_val == NULL) {
        *(int*)ret_val = -1;
        return ret_val;
    }

    int i = 1;
    char * search_path;
    
    while( (search_path = data[i]) != NULL ) {

        int status;
    
        char * pattern = data[0];

        if(( status = find_pattern_in_file_Boyer_Moore(pattern, search_path) ) == 1) {
            printf("Met pattern in \"%s\"!\n", search_path);
        }

        *(ret_val) += status;
        i ++;
    }
    
    return (void *)ret_val;
}

void find_patterns_parallel (path_box pb, const int number_of_threads) {
    printf("Pattern: \"%s\"\n", pb.pattern);

    const int number_of_files  = pb.number_of_names;
    const int thread_data_size = number_of_files / number_of_threads + 3;
    size_t meets               = 0;
    
    pthread_t thread     [number_of_threads];
    void * ret_val       [number_of_threads];
    char * thread_data   [number_of_threads][thread_data_size];
    
    for(int i = 0; i < number_of_threads; ++i) {
                
        thread_data[i][0] = pb.pattern;
        
        int k = 1;
        
        for(int j = i; j < number_of_files; j+=number_of_threads, k++) {
            thread_data[i][k] = pb.names[j];
        }
        thread_data[i][k] = NULL;
    }

    for(int i = 0; i < number_of_threads; ++i) {
        pthread_create(&(thread[i]), NULL, open_file_thread, thread_data[i]);
    }
    
    for(int i = 0; i < number_of_threads; ++i) {
        pthread_join(thread[i], &ret_val[i]);
    }
    for(int i = 0; i < number_of_threads; ++i) {
        if(*(int*)ret_val[i] == -1) {
            fprintf(stderr, "Thread error detected!\n");
            exit(1);
        }
        meets += *(int *)ret_val[i];
    }

    printf("Meets: %lu\n", meets);
}
