#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int total;
    int net;
    int vm;
    int kernel;
    int unsafe;
} sysctl_stats_t;

static char *trim(char *s) {
    while (*s != '\0' && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;

    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }
    return s;
}

static int is_unsafe_kv(const char *key, const char *val) {
    if (strcmp(key, "kernel.kptr_restrict") == 0 && strcmp(val, "0") == 0) {
        return 1;
    }
    if (strcmp(key, "kernel.unprivileged_bpf_disabled") == 0 && strcmp(val, "0") == 0) {
        return 1;
    }
    if (strcmp(key, "fs.protected_hardlinks") == 0 && strcmp(val, "0") == 0) {
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/sysctl.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<sysctl.conf>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    sysctl_stats_t st = {0, 0, 0, 0, 0};

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';

        char *comment = strchr(line, '#');
        if (comment) {
            *comment = '\0';
        }

        char *p = trim(line);
        if (*p == '\0') {
            continue;
        }

        char *eq = strchr(p, '=');
        if (eq == NULL) {
            continue;
        }

        *eq = '\0';
        char *key = trim(p);
        char *val = trim(eq + 1);

        if (key[0] == '\0' || val[0] == '\0') {
            continue;
        }

        st.total++;

        if (strncmp(key, "net.", 4) == 0) st.net++;
        if (strncmp(key, "vm.", 3) == 0) st.vm++;
        if (strncmp(key, "kernel.", 7) == 0) st.kernel++;

        if (is_unsafe_kv(key, val)) {
            st.unsafe++;
        }
    }

    fclose(f);

    printf("total=%d net=%d vm=%d kernel=%d unsafe=%d\n",
           st.total, st.net, st.vm, st.kernel, st.unsafe);

    if (argc == 1) {
        return (st.total == 6 && st.net == 2 && st.vm == 1 && st.kernel == 2 && st.unsafe == 0)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
