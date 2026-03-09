#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int mem_limited;
    uint64_t mem_max;
    uint64_t mem_current;
    int swap_limited;
    uint64_t swap_max;
    double used_pct;
} mem_stats_t;

/*
 * Lee valor cgroup que puede ser entero o literal "max".
 * is_limited=0 cuando el valor es "max".
 */
static int read_cgroup_limit(const char *path, int *is_limited, uint64_t *value) {
    if (path == NULL || is_limited == NULL || value == NULL) {
        return -1;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        return -1;
    }

    char buf[128] = {0};
    if (fscanf(f, "%127s", buf) != 1) {
        fclose(f);
        return -1;
    }
    fclose(f);

    if (strcmp(buf, "max") == 0) {
        *is_limited = 0;
        *value = 0;
        return 0;
    }

    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(buf, &end, 10);
    if (errno != 0 || end == buf || *end != '\0') {
        return -1;
    }

    *is_limited = 1;
    *value = (uint64_t)v;
    return 0;
}

int main(int argc, char **argv) {
    const char *mem_max_path = "tests/data/memory.max.sample";
    const char *mem_cur_path = "tests/data/memory.current.sample";
    const char *swap_max_path = "tests/data/memory.swap.max.sample";

    if (argc == 4) {
        mem_max_path = argv[1];
        mem_cur_path = argv[2];
        swap_max_path = argv[3];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [<memory.max> <memory.current> <memory.swap.max>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    mem_stats_t st;
    memset(&st, 0, sizeof(st));

    if (read_cgroup_limit(mem_max_path, &st.mem_limited, &st.mem_max) != 0) {
        perror("memory.max");
        return EXIT_FAILURE;
    }

    int cur_limited = 0;
    if (read_cgroup_limit(mem_cur_path, &cur_limited, &st.mem_current) != 0 || cur_limited == 0) {
        /* memory.current siempre debe ser numérico, no "max". */
        fprintf(stderr, "memory.current invalido\n");
        return EXIT_FAILURE;
    }

    if (read_cgroup_limit(swap_max_path, &st.swap_limited, &st.swap_max) != 0) {
        perror("memory.swap.max");
        return EXIT_FAILURE;
    }

    if (st.mem_limited && st.mem_max > 0U) {
        st.used_pct = 100.0 * (double)st.mem_current / (double)st.mem_max;
    } else {
        st.used_pct = -1.0;
    }

    printf("mem_limited=%d mem_max=%llu mem_current=%llu used_pct=%.2f swap_limited=%d swap_max=%llu\n",
           st.mem_limited,
           (unsigned long long)st.mem_max,
           (unsigned long long)st.mem_current,
           st.used_pct,
           st.swap_limited,
           (unsigned long long)st.swap_max);

    if (argc == 1) {
        return (st.mem_limited == 1 && st.mem_max == 104857600ULL && st.mem_current == 52428800ULL &&
                st.swap_limited == 0 && st.used_pct > 49.99 && st.used_pct < 50.01)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
