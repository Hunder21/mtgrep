#include "../include/path_box.h"

int get_content_size (char ** dir_content) {
    int count = 0;
    while(dir_content[count] != NULL) count ++;
    return count;
}

char * format_dir_path (char * dir) {
    
    if(dir == NULL) {
        fprintf(stderr, "NULL pointer!");
        exit(1);
    }

    size_t dir_len = strlen(dir);

    if(dir[dir_len] != '\0') {
        printf("Non null-terminated string!");
    }
    
    if(dir_len == 0) {
        fprintf(stderr, "Zero directory len!");
        exit(1);
    }

    char * tmp = NULL;

    if((tmp = malloc( (dir_len+2) * sizeof(char) )) == NULL) {
        fprintf(stderr, "Couldn't reallocate memmory for dir!");
        exit(1);
    }

    memcpy(tmp, dir, dir_len);

    if(dir[dir_len-1] != '/') {
        tmp[dir_len]   = '/';
        tmp[dir_len+1] = '\0';
    }

    return tmp;
}

char ** get_dir_content (char * raw_dir) {
    
    char * dir = format_dir_path(raw_dir);

    if(dir == NULL) {
        fprintf(stderr, "Null dir pointer");
        exit(1);
    }
    
    int dir_len = strlen(dir);
    
    // Directory content
    char ** dir_content;

    if((dir_content = malloc(MIN_RESIZE*sizeof(char *))) == NULL) {
        fprintf(stderr, "Couldn't allocate memmory for content list!");
        exit(1);
    }

    size_t dir_content_size = MIN_RESIZE;
    
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
        char * full_path = NULL;

        if((full_path = malloc((dir_len + strlen(member->d_name) + 1) * sizeof(char))) == NULL) {
            fprintf(stderr, "Couldn't allocate memmory");
            exit(1);
        }
        
        // Copy without dir to full path
        memcpy(full_path, dir, dir_len+1);
        
        // Build full path
        strcat(full_path, member->d_name);
        
        // Full path file status
        if (stat(full_path, &fs) == -1) {
            fprintf(stderr,"Couldn't get stat: %s!\n", full_path);
            exit(1);
        }

        if(S_ISREG(fs.st_mode)) {
            // Procces regular files

            if(count_content == (dir_content_size - 1)) {
                // Add 10 more slots to dir_content
                dir_content_size += MIN_RESIZE;
                if((dir_content = realloc(dir_content, sizeof(char*)*(dir_content_size))) == NULL) {
                    fprintf(stderr, "Couldn't allocate memory for new pointer!\n");
                    exit(1);
                }
            }
            
            if ((dir_content[count_content] = malloc(strlen(full_path)+1)) == NULL) {
                fprintf(stderr, "Couldn't allocate memory for new regular file!\n");
                exit(1);
            }
            
            snprintf(dir_content[count_content], strlen(full_path)+1, "%s", full_path);
            count_content++;
        
        } else if (S_ISDIR(fs.st_mode)) {
            // Process directory
                        
            char ** recursive_dir_content = get_dir_content(full_path);
            
            int count = get_content_size(recursive_dir_content);
            
            if(count + count_content >= dir_content_size) {
                dir_content_size = count + count_content + 1;
                if((dir_content = realloc(dir_content, sizeof(char*)*(dir_content_size))) == NULL) {
                    fprintf(stderr, "Couldn't allocate memory for new pointer!\n");
                    exit(1);
                }
            }
            
            for(int i = 0; i < count; ++i) {
            
                size_t content_len = strlen(recursive_dir_content[i]) + 1;
                
                if ((dir_content[count_content] = malloc(content_len)) == NULL) {
                    fprintf(stderr, "Couldn't allocate memory for new regular file!\n");
                    exit(1);
                }

                memcpy(dir_content[count_content], recursive_dir_content[i], content_len);
                count_content ++;
            
                free(recursive_dir_content[i]);
            }
            
            free(recursive_dir_content);
        }
        
        free(full_path);
    }
    dir_content[count_content]= NULL;
    
    closedir(dr);

    free(dir);
    
    return dir_content;
}

path_box pb_init (int argc, char *argv[]) {

    if(argc < 2) {
        fprintf(stderr, "Pattern isn't specified!\n");
        exit(1);
    }

    path_box pb = {0};
    
    pb.pattern = argv[1];

    if(argc == 2) {
        // If no input files or directories weren't specified, 
        // assume, that we scan current directory
        char ** self_dir_content = get_dir_content("./");
        int content_size         = get_content_size(self_dir_content);
        pb.number_of_names       = content_size;
        pb.names                 = self_dir_content;
    } else {
        
        int pb_allocated_names = MIN_RESIZE;
        pb.names               = malloc(MIN_RESIZE*sizeof(char *));

        for(int i = 2; i < argc; ++i) {
            
            struct stat fs;

            if (stat(argv[i], &fs) == -1) {
                fprintf(stderr, "Couldn't get stat: %s!\n", argv[i]);
                exit(1);
            }

            if(S_ISREG(fs.st_mode)) {
            
                if(pb.number_of_names == pb_allocated_names) {
                    pb_allocated_names += MIN_RESIZE;
                    if ((pb.names = realloc(pb.names, (pb_allocated_names)*(sizeof(char *)))) == NULL) {
                        fprintf(stderr, "Couldn't reallocate memmory for names list!");
                        exit(1);
                    };
                }
                
                if((pb.names[pb.number_of_names] = malloc((strlen(argv[i]) + 1) * sizeof(char))) == NULL) {
                    fprintf(stderr, "Couldn't reallocate memmory!");
                    exit(1);
                }

                memcpy(pb.names[pb.number_of_names], argv[i], strlen(argv[i]) + 1);
                pb.number_of_names++;
            
            } else if (S_ISDIR(fs.st_mode)) {
                char ** dir_content = get_dir_content (argv[i]    );
                int    content_size = get_content_size(dir_content);

                if(content_size > (pb_allocated_names - pb.number_of_names)) {
                    pb_allocated_names = pb.number_of_names + content_size + MIN_RESIZE;
                    if ((pb.names = realloc(pb.names, (pb_allocated_names)*(sizeof(char *)))) == NULL) {
                        fprintf(stderr, "Couldn't reallocate memmory for names list!");
                        exit(1);
                    };
                }

                int last_ptr_idx = pb.number_of_names;

                for(int i = 0; i < content_size; ++i) {
                    pb.names[last_ptr_idx + i] = dir_content[i];
                }

                free(dir_content);
                
                pb.number_of_names += content_size;
            }
        }
    }
    return pb;
}

void pb_destroy (path_box pb) {
    for(int i = 0; i < pb.number_of_names; ++i) {
        free(pb.names[i]);
    }
    free(pb.names);
}
