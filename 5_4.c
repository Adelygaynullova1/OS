#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Недостаточно аргументов");
        return 1;
    }

    FILE *src = fopen(argv[1], "r");
    if (src == NULL) {
        perror("Ошибка открытия исходного файла");
        return 1;
    }

    FILE *dest = fopen(argv[2], "w");
    if (dest == NULL) {
        perror("Ошибка открытия файла назначения");
        fclose(src);
        return 1;
    }

    int ch;
    while ((ch = getc(src)) != EOF) {
        if (putc(ch, dest) == EOF) {
            perror("Ошибка записи в файл");
            fclose(src);
            fclose(dest);
            return 1;
        }
    }

    if (ferror(src)) {
        perror("Ошибка чтения файла");
        fclose(src);
        fclose(dest);
        return 1;
    }

    if (fclose(src) != 0) {
        perror("Ошибка закрытия исходного файла");
        fclose(dest);
        return 1;
    }
    if (fclose(dest) != 0) {
        perror("Ошибка закрытия файла назначения");
        return 1;
    }

    struct stat file_stat;
    if (stat(argv[1], &file_stat) == 0) {
        if (chmod(argv[2], file_stat.st_mode) != 0) {
            perror("Ошибка копирования прав доступа");
            return 1;
        }
    } else {
        perror("Ошибка получения информации о файле");
        return 1;
    }

    printf("Файл '%s' успешно скопирован в '%s'\n", argv[1], argv[2]);
    return 0;
}
