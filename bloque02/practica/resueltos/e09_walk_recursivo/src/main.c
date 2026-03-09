#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/*
 * join_path:
 * construye "base/name" en heap para evitar buffers fijos pequeños.
 */
static char *join_path(const char *base, const char *name) {
    size_t need = strlen(base) + 1 + strlen(name) + 1;
    char *out = malloc(need);
    if (!out) return NULL;
    snprintf(out, need, "%s/%s", base, name);
    return out;
}

static int walk(const char *path) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        perror("lstat");
        return -1;
    }

    printf("%s\n", path);

    /* Solo recursamos si realmente es directorio */
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

    if (walk(base) == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
