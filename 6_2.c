#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>

#define MAX_BYTES 255

void search_in_file(const char *filename, const char *pattern, int pattern_len) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return;
    }

    unsigned char buffer[MAX_BYTES];
    ssize_t bytes_read;
    off_t total_bytes = 0;
    int matches = 0;

    printf("Process %d: Searching in file %s\n", getpid(), filename);

    while ((bytes_read = read(fd, buffer, MAX_BYTES)) > 0) {
        total_bytes += bytes_read;
        for (int i = 0; i <= bytes_read - pattern_len; i++) {
            if (memcmp(buffer + i, pattern, pattern_len) == 0) {
                matches++;
            }
        }
    }

    printf("Process %d: File %s, Total bytes read: %ld, Matches found: %d\n", getpid(), filename, total_bytes, matches);

    close(fd);
}

int main() {
    char dir_path[256];
    char pattern[MAX_BYTES];
    int pattern_len;
    int max_processes;

    printf("Enter directory path: ");
    scanf("%s", dir_path);
    printf("Enter byte pattern to search (up to %d bytes): ", MAX_BYTES - 1);
    scanf("%s", pattern);
    pattern_len = strlen(pattern);
    printf("Enter maximum number of simultaneous processes: ");
    scanf("%d", &max_processes);

    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    pid_t pids[max_processes];
    int active_processes = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, entry->d_name);

            if (active_processes >= max_processes) {
                pid_t finished_pid = waitpid(-1, NULL, 0);
                if (finished_pid > 0) {
                    active_processes--;
                }
            }

            pid_t pid = fork();
            if (pid == 0) {
                search_in_file(filepath, pattern, pattern_len);
                exit(0);
            } else if (pid > 0) {
                pids[active_processes++] = pid;
            } else {
                perror("fork");
            }
        }
    }

    while (active_processes > 0) {
        pid_t finished_pid = waitpid(-1, NULL, 0);
        if (finished_pid > 0) {
            active_processes--;
        }
    }

    closedir(dir);
    return 0;
}
