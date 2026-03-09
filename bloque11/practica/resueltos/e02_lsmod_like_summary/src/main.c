#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int modules;
    int busy;
    unsigned long top_size;
    char top_module[128];
} lsmod_stats_t;

static int parse_lsmod_file(const char *path, lsmod_stats_t *out) {
    if (path == NULL || out == NULL) {
        return -1;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        return -1;
    }

    memset(out, 0, sizeof(*out));

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        /* Saltamos header tipo: "Module Size Used by" */
        if (strncmp(line, "Module", 6) == 0) {
            continue;
        }

        char module[128] = {0};
        unsigned long size = 0UL;
        long used = 0L;

        int n = sscanf(line, "%127s %lu %ld", module, &size, &used);
        if (n != 3) {
            continue;
        }

        out->modules++;

        if (used > 0L) {
            out->busy++;
        }

        if (size > out->top_size) {
            out->top_size = size;
            snprintf(out->top_module, sizeof(out->top_module), "%s", module);
        }
    }

    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/lsmod.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<lsmod_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    lsmod_stats_t st;
    if (parse_lsmod_file(path, &st) != 0) {
        perror("parse_lsmod_file");
        return EXIT_FAILURE;
    }

    printf("modules=%d busy=%d top=%s top_size=%lu\n",
           st.modules, st.busy, st.top_module[0] ? st.top_module : "none", st.top_size);

    if (argc == 1) {
        return (st.modules == 3 && st.busy == 2 && strcmp(st.top_module, "xfs") == 0 &&
                st.top_size == 1765376UL)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
