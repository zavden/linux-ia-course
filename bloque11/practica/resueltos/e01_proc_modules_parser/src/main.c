#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int modules;
    int in_use;
    int dep_edges;
    int states_live;
} mod_stats_t;

/*
 * Cuenta elementos en lista de dependencias separada por comas.
 * En /proc/modules, '-' significa "sin dependencias".
 */
static int count_deps(const char *deps_field) {
    if (deps_field == NULL || deps_field[0] == '\0' || strcmp(deps_field, "-") == 0) {
        return 0;
    }

    int n = 0;
    const char *p = deps_field;

    while (*p != '\0') {
        while (*p == ',') {
            p++;
        }
        if (*p == '\0') {
            break;
        }

        n++;
        while (*p != '\0' && *p != ',') {
            p++;
        }
    }

    return n;
}

static int parse_proc_modules(const char *path, mod_stats_t *out) {
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
        char module[128] = {0};
        unsigned long size = 0UL;
        long use_count = 0L;
        char deps[512] = {0};
        char state[32] = {0};

        /*
         * Formato esperado aproximado:
         * <name> <size> <use_count> <deps> <state> <address>
         */
        int n = sscanf(line, "%127s %lu %ld %511s %31s", module, &size, &use_count, deps, state);
        if (n < 5) {
            continue;
        }

        (void)size;
        out->modules++;

        if (use_count > 0L) {
            out->in_use++;
        }

        out->dep_edges += count_deps(deps);

        if (strcmp(state, "Live") == 0) {
            out->states_live++;
        }
    }

    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/proc_modules.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<proc_modules_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    mod_stats_t st;
    if (parse_proc_modules(path, &st) != 0) {
        perror("parse_proc_modules");
        return EXIT_FAILURE;
    }

    printf("modules=%d in_use=%d dep_edges=%d live=%d\n",
           st.modules, st.in_use, st.dep_edges, st.states_live);

    if (argc == 1) {
        return (st.modules == 3 && st.in_use == 2 && st.dep_edges == 1 && st.states_live == 3)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
