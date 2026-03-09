#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/*
 * Reto C03:
 * Esta plantilla marca arquitectura sugerida,
 * pero te deja implementar la lógica completa.
 */

struct filters {
    const char *name_glob;
    char type;
    int use_size;
    long size_bytes;
    int use_maxdepth;
    int maxdepth;
    int use_mindepth;
    int mindepth;
};

/* TODO 1: parseo CLI robusto para todas las opciones */
static int parse_args(int argc, char **argv, const char **base_out, struct filters *f) {
    (void)argc;
    (void)argv;
    *base_out = ".";
    memset(f, 0, sizeof(*f));
    return 0;
}

/* TODO 2: join_path seguro */
static char *join_path(const char *a, const char *b) {
    size_t n = strlen(a) + 1 + strlen(b) + 1;
    char *p = malloc(n);
    if (!p) return NULL;
    snprintf(p, n, "%s/%s", a, b);
    return p;
}

/* TODO 3: funciones de matching por filtro */
static int match_name(const char *name, const struct filters *f) {
    if (!f->name_glob) return 1;
    return fnmatch(f->name_glob, name, 0) == 0;
}

/* TODO 4: completar el resto de filters (type/size/depth) */

/* TODO 5: recorrido recursivo con depth y manejo de errores */
static int walk(const char *path, const char *name_only, int depth, const struct filters *f, int *had_error) {
    (void)name_only;
    (void)depth;
    (void)f;
    (void)had_error;

    struct stat st;
    if (lstat(path, &st) == -1) {
        fprintf(stderr, "minifind-plus: %s: %s\n", path, strerror(errno));
        return 0;
    }

    /* Uso mínimo de matching por nombre para mantener la plantilla ejecutable */
    if (match_name(name_only, f)) {
        printf("%s\n", path);
    }

    if (!S_ISDIR(st.st_mode)) return 0;

    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "minifind-plus: %s: %s\n", path, strerror(errno));
        return 0;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        char *child = join_path(path, ent->d_name);
        if (!child) {
            perror("malloc");
            closedir(dir);
            return -1;
        }
        if (walk(child, ent->d_name, depth + 1, f, had_error) == -1) {
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
    const char *base = ".";
    struct filters f;
    int had_error = 0;

    if (parse_args(argc, argv, &base, &f) != 0) {
        return EXIT_FAILURE;
    }

    if (walk(base, base, 0, &f, &had_error) == -1) {
        return EXIT_FAILURE;
    }

    return had_error ? EXIT_FAILURE : EXIT_SUCCESS;
}
