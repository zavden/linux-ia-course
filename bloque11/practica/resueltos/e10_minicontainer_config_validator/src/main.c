#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char rootfs[512];
    char cmd[512];
    char mem_max[64];
    char cpu_max[128];
    char namespaces[256];
    char seccomp_profile[512];

    int have_rootfs;
    int have_cmd;
    int have_mem_max;
    int have_cpu_max;
    int have_namespaces;
    int have_seccomp;
} mini_cfg_t;

static char *trim(char *s) {
    while (*s != '\0' && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;

    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }
    return s;
}

static int parse_u64(const char *s, uint64_t *out) {
    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') {
        return -1;
    }
    *out = (uint64_t)v;
    return 0;
}

/* Busca token exacto en CSV simple: "pid,net,mnt" */
static int csv_has_token(const char *csv, const char *token) {
    size_t tlen = strlen(token);
    const char *p = csv;

    while (*p != '\0') {
        while (*p == ',' || isspace((unsigned char)*p)) {
            p++;
        }

        const char *start = p;
        while (*p != '\0' && *p != ',') {
            p++;
        }

        const char *end = p;
        while (end > start && isspace((unsigned char)end[-1])) {
            end--;
        }

        size_t len = (size_t)(end - start);
        if (len == tlen && strncmp(start, token, tlen) == 0) {
            return 1;
        }
    }

    return 0;
}

static int valid_cpu_max(const char *value) {
    char copy[128];
    if (snprintf(copy, sizeof(copy), "%s", value) >= (int)sizeof(copy)) {
        return 0;
    }

    char a[64] = {0};
    char b[64] = {0};
    if (sscanf(copy, "%63s %63s", a, b) != 2) {
        return 0;
    }

    uint64_t period = 0;
    if (parse_u64(b, &period) != 0 || period == 0U) {
        return 0;
    }

    if (strcmp(a, "max") == 0) {
        return 1;
    }

    uint64_t quota = 0;
    if (parse_u64(a, &quota) != 0 || quota == 0U) {
        return 0;
    }

    return 1;
}

static int ends_with(const char *s, const char *suffix) {
    size_t ls = strlen(s);
    size_t lf = strlen(suffix);
    if (lf > ls) return 0;
    return (strcmp(s + (ls - lf), suffix) == 0) ? 1 : 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/minicontainer.good.conf";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<config_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    mini_cfg_t cfg;
    memset(&cfg, 0, sizeof(cfg));

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

        if (strcmp(key, "rootfs") == 0) {
            cfg.have_rootfs = 1;
            snprintf(cfg.rootfs, sizeof(cfg.rootfs), "%s", val);
            continue;
        }

        if (strcmp(key, "cmd") == 0) {
            cfg.have_cmd = 1;
            snprintf(cfg.cmd, sizeof(cfg.cmd), "%s", val);
            continue;
        }

        if (strcmp(key, "mem_max") == 0) {
            cfg.have_mem_max = 1;
            snprintf(cfg.mem_max, sizeof(cfg.mem_max), "%s", val);
            continue;
        }

        if (strcmp(key, "cpu_max") == 0) {
            cfg.have_cpu_max = 1;
            snprintf(cfg.cpu_max, sizeof(cfg.cpu_max), "%s", val);
            continue;
        }

        if (strcmp(key, "namespaces") == 0) {
            cfg.have_namespaces = 1;
            snprintf(cfg.namespaces, sizeof(cfg.namespaces), "%s", val);
            continue;
        }

        if (strcmp(key, "seccomp_profile") == 0) {
            cfg.have_seccomp = 1;
            snprintf(cfg.seccomp_profile, sizeof(cfg.seccomp_profile), "%s", val);
            continue;
        }
    }

    fclose(f);

    int present = cfg.have_rootfs + cfg.have_cmd + cfg.have_mem_max +
                  cfg.have_cpu_max + cfg.have_namespaces + cfg.have_seccomp;
    int required = 6;

    int valid = 1;

    if (!cfg.have_rootfs || cfg.rootfs[0] != '/') {
        valid = 0;
    }

    if (!cfg.have_cmd || cfg.cmd[0] != '/') {
        valid = 0;
    }

    if (!cfg.have_mem_max) {
        valid = 0;
    } else {
        uint64_t mem = 0;
        if (parse_u64(cfg.mem_max, &mem) != 0 || mem < 16777216ULL || mem > 8589934592ULL) {
            valid = 0;
        }
    }

    if (!cfg.have_cpu_max || !valid_cpu_max(cfg.cpu_max)) {
        valid = 0;
    }

    if (!cfg.have_namespaces || !csv_has_token(cfg.namespaces, "pid") ||
        !csv_has_token(cfg.namespaces, "mnt")) {
        valid = 0;
    }

    if (!cfg.have_seccomp || cfg.seccomp_profile[0] != '/' ||
        !ends_with(cfg.seccomp_profile, ".json")) {
        valid = 0;
    }

    printf("required=%d present=%d missing=%d valid=%d\n",
           required, present, required - present, valid);

    return valid ? EXIT_SUCCESS : EXIT_FAILURE;
}
