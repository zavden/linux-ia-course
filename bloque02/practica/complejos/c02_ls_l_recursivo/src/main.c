#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/*
 * Reto C02:
 * Recrear ls -l recursivo con formato claro.
 */

/* TODO 1: convertir permisos a cadena rwxrwxrwx */
static void mode_to_string(mode_t mode, char out[11]) {
    (void)mode;
    strcpy(out, "----------");
}

/* TODO 2: join_path seguro */
static char *join_path(const char *base, const char *name) {
    size_t n = strlen(base) + 1 + strlen(name) + 1;
    char *p = malloc(n);
    if (!p) return NULL;
    snprintf(p, n, "%s/%s", base, name);
    return p;
}

/* TODO 3: imprimir una entrada con formato tipo ls -l */
static void print_entry(const char *path, const struct stat *st) {
    char perm[11];
    mode_to_string(st->st_mode, perm);
    printf("%s %s\n", perm, path);
}

/* TODO 4: implementar recursión robusta */
static int walk(const char *path) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        perror("lstat");
        return -1;
    }

    print_entry(path, &st);

    if (!S_ISDIR(st.st_mode)) {
        return 0;
    }

    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return -1;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        char *child = join_path(path, ent->d_name);
        if (!child) {
            perror("malloc");
            closedir(dir);
            return -1;
        }

        if (walk(child) == -1) {
            free(child);
            closedir(dir);
            return -1;
        }

        free(child);
    }

    closedir(dir);
    return 0;
}

int main(int argc, char **argv) {
    const char *base = (argc >= 2) ? argv[1] : ".";
    return walk(base) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
