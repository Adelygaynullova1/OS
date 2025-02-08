#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <time.h>

void search_files(const char *dir_path, FILE *output_file, off_t min_size, off_t max_size, time_t min_time, time_t max_time) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        fprintf(stderr, "Ошибка открытия каталога '%s': %s\n", dir_path, strerror(errno));
        return;
    }

    struct dirent *entry;
    struct stat file_stat;
    char full_path[1024];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (stat(full_path, &file_stat) == -1) {
            fprintf(stderr, "Ошибка получения информации о файле '%s': %s\n", full_path, strerror(errno));
            continue;
        }

        if (S_ISDIR(file_stat.st_mode)) {
            search_files(full_path, output_file, min_size, max_size, min_time, max_time); 
        } else {
            if (file_stat.st_size >= min_size && file_stat.st_size <= max_size &&
                file_stat.st_ctime >= min_time && file_stat.st_ctime <= max_time) {
                char time_str[20];
                struct tm *tm_info = localtime(&file_stat.st_ctime); //содержит разбитое на компоненты время (год, месяц, день, час, минута, секунда).
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

                printf("%s | Размер: %ld байт | Дата создания: %s\n", full_path, file_stat.st_size, time_str);

                fprintf(output_file, "%s | Размер: %ld байт | Дата создания: %s\n", full_path, file_stat.st_size, time_str);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        printf("Недостаточно аргументов");
        return 1;
    }

    const char *dir_path = argv[1];
    const char *output_filename = argv[2];
    off_t min_size = atol(argv[3]);
    off_t max_size = atol(argv[4]);

    struct tm tm1 = {0}, tm2 = {0};
    strptime(argv[5], "%Y-%m-%d", &tm1);
    strptime(argv[6], "%Y-%m-%d", &tm2);
    time_t min_time = mktime(&tm1);
    time_t max_time = mktime(&tm2);

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file) {
        fprintf(stderr, "Ошибка открытия файла '%s': %s\n", output_filename, strerror(errno));
        return 1;
    }

    search_files(dir_path, output_file, min_size, max_size, min_time, max_time);

    fclose(output_file);
    printf("Результаты записаны в '%s'\n", output_filename);
    return 0;
}
