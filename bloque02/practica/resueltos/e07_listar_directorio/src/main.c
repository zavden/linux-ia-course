#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* Tipo compacto similar a ls */
static char type_char(mode_t mode) {
    if (S_ISREG(mode)) return '-';
    if (S_ISDIR(mode)) return 'd';
    if (S_ISLNK(mode)) return 'l';
    return '?';
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <directorio>\n", argv[0]);
        return EXIT_FAILURE;
    }

    DIR *dir = opendir(argv[1]);
    if (!dir) {
        perror("opendir");
        return EXIT_FAILURE;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        /* Ignoramos entradas especiales para no ensuciar salida */
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        /* Construcción segura del path completo */
        size_t need = strlen(argv[1]) + 1 + strlen(ent->d_name) + 1;
        char *full = malloc(need);
        if (!full) {
            perror("malloc");
            closedir(dir);
            return EXIT_FAILURE;
        }
        snprintf(full, need, "%s/%s", argv[1], ent->d_name);

        struct stat st;
        if (lstat(full, &st) == -1) {
            perror("lstat");
            free(full);
            closedir(dir);
            return EXIT_FAILURE;
        }

        printf("%c %8lld %s\n", type_char(st.st_mode), (long long)st.st_size, ent->d_name);
        free(full);
    }

    closedir(dir);
    return EXIT_SUCCESS;
}
