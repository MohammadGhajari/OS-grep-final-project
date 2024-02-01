#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>

#define MAX_PATH_LENGTH 256
#define MAX_PATTERN_LENGTH 256
#define MAX_LINE_LENGTH 1024

pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t result2_mutex = PTHREAD_MUTEX_INITIALIZER;
int* total_files_searched;
//(*total_files_searched) = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
//(*total_files_searched) = 0;
int total_matches_found = 0;

struct ThreadArgs {
    char filename[MAX_PATH_LENGTH];
    char pattern[MAX_PATTERN_LENGTH];
};

void* search_file(void* args) {
    struct ThreadArgs* thread_args = (struct ThreadArgs*)args;
    FILE* file = fopen(thread_args->filename, "r");

    if (file == NULL) {
        perror("Error opening file");
        pthread_exit(NULL);
    }

    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    int matches_in_file = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        line_number++;
        char* pos = strstr(line, thread_args->pattern);
        if (pos != NULL) {
            pthread_mutex_lock(&result_mutex);
            printf("%s:%d:%ld %s", thread_args->filename, line_number, (long)(pos - line) + 1, line);
            //total_matches_found++;
            pthread_mutex_unlock(&result_mutex);
            matches_in_file++;
        }
    }

    fclose(file);
    pthread_exit((void*)(intptr_t)matches_in_file); // Return the number of matches found in this file
}

void search_directory(char* path, char* pattern, int pipe_fd) {
    DIR* dir = opendir(path);

    if (dir == NULL) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {

            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {

                pid_t child_pid = fork();

                if (child_pid == -1) {
                    perror("Error forking process");
                    exit(EXIT_FAILURE);
                }

                if (child_pid == 0) {  // Child process
                    char subdir[MAX_PATH_LENGTH];
                    snprintf(subdir, sizeof(subdir), "%s/%s", path, entry->d_name);
                    search_directory(subdir, pattern, pipe_fd); // Recursive call in child process
                    exit(EXIT_SUCCESS);
                }


            }
        } else if (entry->d_type == DT_REG) {
            pthread_t thread;
            struct ThreadArgs* args = (struct ThreadArgs*)malloc(sizeof(struct ThreadArgs));
            snprintf(args->filename, sizeof(args->filename), "%s/%s", path, entry->d_name);
            snprintf(args->pattern, sizeof(args->pattern), "%s", pattern);

            if (pthread_create(&thread, NULL, search_file, (void*)args) != 0) {
                perror("Error creating thread");
                exit(EXIT_FAILURE);
            }

            int matches_in_file;
            pthread_join(thread, (void**)&matches_in_file);
            free(args);
            pthread_mutex_lock(&result2_mutex);
            (*total_files_searched)++;
            pthread_mutex_unlock(&result2_mutex);

            // Send the number of matches found in the file to the parent process via pipe
            write(pipe_fd, &matches_in_file, sizeof(matches_in_file));
        }
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <pattern>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* path = argv[1];
    char* pattern = argv[2];


    total_files_searched = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *total_files_searched = 0;

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    search_directory(path, pattern, pipe_fd[1]); // Pass the write end of the pipe to the initial call

    close(pipe_fd[1]); // Close the write end of the pipe in the parent process

    int matches_in_file;
    while (read(pipe_fd[0], &matches_in_file, sizeof(matches_in_file)) > 0) {
        //printf("%d\n", matches_in_file);
        total_matches_found += matches_in_file; // Accumulate the match counts from child processes
        //printf("total match:::: %d\n", total_matches_found);
    }

    close(pipe_fd[0]); // Close the read end of the pipe

    printf("Total files searched: %d\n", *total_files_searched);
    printf("Total matches found: %d\n", total_matches_found);

    return 0;
}
