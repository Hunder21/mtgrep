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
            fprintf(stderr, "Couldn't get stat!");
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
                    fprintf(stderr, "Couldn't get stat!");
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

char ** get_dir_content (char * dir) {
    // Directory content
    char ** dir_content = malloc(sizeof(char *));
    if(dir_content == NULL) {
        fprintf(stderr, "Couldn't allocate memory for directory content!");
        exit(1);
    }
    // Count directory content
    size_t count_content = 0;
    // Open directory
    DIR * dr = opendir(dir);
    if(dr == NULL) {
        fprintf(stderr, "Couldn't open directory: %s!", dir);
        exit(1);
    }
    // Directory member status
    struct dirent * member;
    struct stat fs;

    while((member = readdir(dr)) != NULL) {
        // Ignore . and ..
        if (strcmp(member->d_name, ".") == 0 || strcmp(member->d_name, "..") == 0)
            continue;
        // Get full path
        char full_path [MAX_ARG_WIDTH];
        snprintf(full_path, MAX_ARG_WIDTH, "%s", dir);
        
        if(strlen(full_path) + strlen(member->d_name) + 1 > MAX_ARG_WIDTH) {
            fprintf(stderr, "Path size overflowed!");
            exit(1);
        } else {
            strcat(full_path, member->d_name);
        }
        
        // Full path file status
        if (stat(full_path, &fs) == -1) {
            fprintf(stderr,"Couldn't get stat!");
            exit(1);
        }
        
        // Add only regular dir content (No recursive dir search now)
        if(S_ISREG(fs.st_mode)) {
            if((dir_content = realloc(dir_content, sizeof(char*)*(count_content+2))) == NULL) {
                fprintf(stderr, "Couldn't allocate memory for new pointer!");
                exit(1);
            }

            if ((*(dir_content + count_content) = malloc(strlen(member->d_name)+1)) == NULL) {
                fprintf(stderr, "Couldn't allocate memory for new regular file!");
                exit(1);
            }
            
            snprintf(*(dir_content + count_content), MAX_ARG_WIDTH, "%s", member->d_name);
            count_content++;
        }
    }
    *(dir_content + count_content) = NULL;
    closedir(dr);
    return dir_content;
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
    char sliding_window [pattern_len+1];

    if(fd == NULL) {
        fprintf(stderr, "Warning: Couldn't open file: %s!", abs_path);
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
        fprintf(stderr, "Couldn't open file: %s!", abs_path);
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

void * open_file_thread (void * search_data) {
    int * ret_val = calloc(1, sizeof(int));
    if(ret_val == NULL) {
        *(int*)ret_val = -1;
        return ret_val;
    }

    for(int i = 0; i < ((search_t *)search_data)->number_of_names; ++i) {
        int status;
        if((status = find_pattern_in_file_Boyer_Moore(((search_t *)search_data)->pattern, ((search_t *)search_data)->names[i])) == 1) {
            printf("Met pattern in \"%s\"!\n", ((search_t *)search_data)->names[i]);
        }
        *(ret_val) += status;
        }
    return (void *)ret_val;
}

void find_patterns_parallel (search_t args, const int number_of_threads) {
    printf("Pattern: \"%s\"\n", args.pattern);

    const int number_of_files = args.number_of_names;
    size_t meets              = 0;
    
    pthread_t thread     [number_of_threads];
    void * ret_val       [number_of_threads];
    search_t search_data [number_of_threads];
    
    for(int i = 0; i < number_of_threads; ++i) {
        int k = 0;
        snprintf(search_data[i].pattern, MAX_ARG_WIDTH, "%s", args.pattern);
        for(int j = i; j < number_of_files; j+=number_of_threads, k++) {
            snprintf(search_data[i].names[k], MAX_ARG_WIDTH, "%s", args.names[j]);
        }
        search_data[i].number_of_names = k;
    }

    for(int i = 0; i < number_of_threads; ++i) {
        pthread_create(&(thread[i]), NULL, open_file_thread, &search_data[i]);
    }
    
    for(int i = 0; i < number_of_threads; ++i) {
        pthread_join(thread[i], &ret_val[i]);
    }
    for(int i = 0; i < number_of_threads; ++i) {
        if(*(int*)ret_val[i] == -1) {
            fprintf(stderr, "Thread error detected!");
            exit(1);
        }
        meets += *(int *)ret_val[i];
    }

    printf("Meets: %lu\n", meets);
}

