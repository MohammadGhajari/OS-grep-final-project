#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

//comment that i add


pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;

// Structure to hold arguments for thread function
typedef struct {
    char filename[256];  // File to search
    char pattern[256];  // Pattern to search for in the file
} ThreadArgs;


long get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000000L + ts.tv_nsec;
}


// Function executed by each thread to search for pattern in a file
void* search_file(void* args) {
    ThreadArgs* thread_args = (ThreadArgs*)args;
    FILE* file = fopen(thread_args->filename, "r");  // Open the file for reading

    if (file == NULL) {
        printf("error in oppening file");
        perror("Error opening file");  // Print error if unable to open file
        pthread_exit(NULL);  // Terminate thread
    }

    pthread_mutex_lock(&result_mutex);
    // Indicates start of file processing
    printf("@");//print this symbol to count the number of searched files
    pthread_mutex_unlock(&result_mutex);

    char line[1024];
    int line_number = 0;
    int matches_in_file = 0;

    pthread_t self = pthread_self();

    // Get start time
    long start_time = get_time_ns();

    int flag = 0;

    // Read each line of the file
    while (fgets(line, sizeof(line), file) != NULL) {
        line_number++;
        char* pos = strstr(line, thread_args->pattern);  // Check if pattern exists in line
        if (pos != NULL) {
            flag = 1;
            pthread_mutex_lock(&result_mutex);
            // Print filename, line number, and position of match
            printf("%s:%d.%ld  ID: %lu  ", thread_args->filename, line_number, (long)(pos - line) + 1, self);

            // Indicates occurrence of match
            printf("&");//print this symbol to count the number of matches

            matches_in_file++;
            pthread_mutex_unlock(&result_mutex);
        }
    }

    fclose(file);  // Close the file



    long end_time = get_time_ns();

    long duration = end_time - start_time;

    if(flag) printf("Duration: %ld ns|\n", duration);




    pthread_exit(NULL);  // Terminate thread
}

// Function to recursively search directories for files matching pattern
void search_directory(char* path, char* pattern) {
    DIR* dir = opendir(path);  // Open directory for reading
    if (dir == NULL) {
        perror("Error opening directory");  // Print error if unable to open directory
        printf("No such directory");
        exit(EXIT_FAILURE);  // Exit program with failure status
    }

    struct dirent* entry = readdir(dir);  // Read directory entries
    int files_searched = 0;

    while (entry != NULL) {
        if (entry->d_type == DT_DIR) {  // If entry is a directory
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                pid_t child_pid = fork();  // Fork a new process

                if (child_pid == -1) {
                    printf("Error forking process");
                    perror("Error forking process");  // Print error if unable to fork process
                    exit(EXIT_FAILURE);  // Exit program with failure status
                }

                if (child_pid == 0) {  // Child process
                    char subdir[256];
                    snprintf(subdir, sizeof(subdir), "%s/%s", path, entry->d_name);
                    search_directory(subdir, pattern);  // Recursive call to search subdirectory
                    exit(files_searched);  // Exit child process with number of files searched
                } else {  // Parent process
                    waitpid(child_pid, &files_searched, 0);  // Wait for child process to finish
                    files_searched += WEXITSTATUS(files_searched);  // Get exit status of child process
                }
            }
        } else if (entry->d_type == DT_REG) {  // If entry is a regular file
            pthread_t thread;
            ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));
            snprintf(args->filename, sizeof(args->filename), "%s/%s", path, entry->d_name);  // Set filename
            snprintf(args->pattern, sizeof(args->pattern), "%s", pattern);  // Set pattern

            if (pthread_create(&thread, NULL, search_file, (void*)args) != 0) {
                perror("Error creating thread");  // Print error if unable to create thread
                exit(EXIT_FAILURE);  // Exit program with failure status
            }

            // Wait for thread to finish
            pthread_join(thread, NULL);
            free(args);  // Free allocated memory for thread arguments
        }

        entry = readdir(dir);  // Read next directory entry
    }

    closedir(dir);  // Close the directory
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <pattern>\n", argv[0]);
        exit(EXIT_FAILURE);  // Exit program with failure status if incorrect number of arguments provided
    }

    char* path = argv[1];  // Directory path
    char* pattern = argv[2];  // Pattern to search for

    // Start searching directory for pattern
    search_directory(path, pattern);

    return 0;
}