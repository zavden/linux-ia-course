#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* Filtros parseados de CLI */
struct filters {
    const char *name;
    char type;             /* 'f', 'd', 'l' o 0 */
    int use_size;
    long size_bytes;
};

/* Unión segura de rutas */
static char *join_path(const char *base, const char *name) {
    size_t need = strlen(base) + 1 + strlen(name) + 1;
    char *out = malloc(need);
    if (!out) return NULL;
    snprintf(out, need, "%s/%s", base, name);
    return out;
}

/* Tipo por st_mode -> letra estilo find */
static char mode_to_type(mode_t mode) {
    if (S_ISREG(mode)) return 'f';
    if (S_ISDIR(mode)) return 'd';
    if (S_ISLNK(mode)) return 'l';
    return '?';
}

/*
 * matches:
 * encapsula TODA la lógica de filtrado para mantener walk limpio.
 */
static int matches(const char *path, const char *name_only, const struct stat *st, const struct filters *f) {
    (void)path;

    if (f->name && strcmp(name_only, f->name) != 0) {
        return 0;
    }

    if (f->type) {
        char t = mode_to_type(st->st_mode);
        if (t != f->type) {
            return 0;
        }
    }

    if (f->use_size) {
        if (st->st_size != f->size_bytes) {
            return 0;
        }
    }

    return 1;
}

/* Recorrido recursivo */
static int walk(const char *path, const char *name_only, const struct filters *f, int *had_error) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        fprintf(stderr, "minifind: %s: %s\n", path, strerror(errno));
        *had_error = 1;
        return 0; /* continuamos escaneo global */
    }

    if (matches(path, name_only, &st, f)) {
        printf("%s\n", path);
    }

    if (!S_ISDIR(st.st_mode)) {
        return 0;
    }

    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "minifind: %s: %s\n", path, strerror(errno));
        *had_error = 1;
        return 0;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        char *child = join_path(path, ent->d_name);
        if (!child) {
            perror("malloc");
            *had_error = 1;
            closedir(dir);
            return -1;
        }

        if (walk(child, ent->d_name, f, had_error) == -1) {
            free(child);
            closedir(dir);
            return -1;
        }

        free(child);
    }

    closedir(dir);
    return 0;
}

static int parse_size_c(const char *s, long *out) {
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (end == s) return -1;
    if (*end != 'c' || end[1] != '\0') return -1;
    if (v < 0) return -1;
    *out = v;
    return 0;
}

int main(int argc, char **argv) {
    struct filters f;
    f.name = NULL;
    f.type = 0;
    f.use_size = 0;
    f.size_bytes = 0;

    const char *base = ".";

    int i = 1;

    /* Primer argumento no-opción se toma como base */
    if (i < argc && argv[i][0] != '-') {
        base = argv[i];
        i++;
    }

    /* Parseo simple de pares opción-valor */
    while (i < argc) {
        if (strcmp(argv[i], "-name") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Falta valor para -name\n");
                return EXIT_FAILURE;
            }
            f.name = argv[i + 1];
            i += 2;
        } else if (strcmp(argv[i], "-type") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Falta valor para -type\n");
                return EXIT_FAILURE;
            }
            if (strlen(argv[i + 1]) != 1) {
                fprintf(stderr, "-type debe ser f, d o l\n");
                return EXIT_FAILURE;
            }
            f.type = argv[i + 1][0];
            if (f.type != 'f' && f.type != 'd' && f.type != 'l') {
                fprintf(stderr, "-type debe ser f, d o l\n");
                return EXIT_FAILURE;
            }
            i += 2;
        } else if (strcmp(argv[i], "-size") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Falta valor para -size\n");
                return EXIT_FAILURE;
            }
            long n = 0;
            if (parse_size_c(argv[i + 1], &n) != 0) {
                fprintf(stderr, "Formato inválido para -size. Usa Nc, ejemplo 20c\n");
                return EXIT_FAILURE;
            }
            f.use_size = 1;
            f.size_bytes = n;
            i += 2;
        } else {
            fprintf(stderr, "Opción desconocida: %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

    int had_error = 0;
    if (walk(base, base, &f, &had_error) == -1) {
        return EXIT_FAILURE;
    }

    return had_error ? EXIT_FAILURE : EXIT_SUCCESS;
}
