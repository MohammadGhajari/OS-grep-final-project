#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>


pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

int* file_count;
int match_count = 0;

typedef struct{
    char name[250];
    char pattern[250];
} ThreadArgs;


void* search_file(void* args) {
    ThreadArgs* thread_args = (ThreadArgs*)args;
    FILE* file = fopen(thread_args->name, "r");

    if (file == NULL) {
        perror("Error in opening file");
        pthread_exit(NULL);
    }

    char line[1000];
    int line_number = 0;
    int matches_word = 0;


    pthread_t threadSelf = pthread_self();


    struct timespec t;
    clock_gettime(0, &t);
    long start =  t.tv_sec * 1000000000L + t.tv_nsec;

    int is_matched = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        line_number++;
        char* pos = strstr(line, thread_args->pattern);
        if (pos != NULL) {
            is_matched = 1;
            pthread_mutex_lock(&mutex1);
            printf("%s:%d:%ld ", thread_args->name, line_number, (long)(pos - line) + 1);
            pthread_mutex_unlock(&mutex1);
            matches_word++;
        }
    }

    fclose(file);


    clock_gettime(0, &t);
    long end =  t.tv_sec * 1000000000L + t.tv_nsec;

    if(is_matched) printf("ID: %lu Duration: %ld ns\n", threadSelf, end - start);

    pthread_exit((void*)(intptr_t)matches_word);
}

void proccessFile (int pipe_fd, char* path, char* pattern, struct dirent* entry) {
    pthread_t thread;
    ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));
    snprintf(args->name, sizeof(args->name), "%s/%s", path, entry->d_name);
    snprintf(args->pattern, sizeof(args->pattern), "%s", pattern);

    if (pthread_create(&thread, NULL, search_file, (void*)args) != 0) {
        perror("Error creating thread");
        exit(1);
    }

    int matches_in_file;
    pthread_join(thread, (void**)&matches_in_file);
    free(args);
    pthread_mutex_lock(&mutex2);
    (*file_count)++;
    pthread_mutex_unlock(&mutex2);

    write(pipe_fd, &matches_in_file, sizeof(matches_in_file));
}

void searchPath(char* path, char* pattern, int pipe_fd) {
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
                    exit(1);
                }

                if (child_pid == 0) {
                    char subdir[250];
                    snprintf(subdir, sizeof(subdir), "%s/%s", path, entry->d_name);
                    searchPath(subdir, pattern, pipe_fd);
                    exit(0);
                }

            }
        } else if (entry->d_type == DT_REG) {
            proccessFile (pipe_fd, path, pattern, entry);
        }
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <pattern>\n", argv[0]);
        exit(1);
    }

    char* path = argv[1];
    char* pattern = argv[2];


    file_count = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *file_count = 0;

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        exit(1);
    }

    searchPath(path, pattern, pipe_fd[1]);

    close(pipe_fd[1]);

    int matches_in_file;
    while (read(pipe_fd[0], &matches_in_file, sizeof(matches_in_file)) > 0) {
        match_count += matches_in_file;
    }

    close(pipe_fd[0]);

    printf("Total files searched: %d\n", *file_count);
    printf("Total matches found: %d", match_count);

    return 0;
}

