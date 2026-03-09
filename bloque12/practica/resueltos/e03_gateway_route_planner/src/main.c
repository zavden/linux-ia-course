#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BE 32
#define MAX_RT 64

typedef struct {
    char name[64];
    int up;
} backend_t;

typedef struct {
    char prefix[128];
    char backend[64];
} route_t;

typedef struct {
    backend_t be[MAX_BE];
    size_t n_be;
    route_t rt[MAX_RT];
    size_t n_rt;
} cfg_t;

static int parse_cfg(const char *path, cfg_t *cfg) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    memset(cfg, 0, sizeof(*cfg));

    char line[512];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        char kind[32], a[128], b[128];
        int n = sscanf(line, "%31s %127s %127s", kind, a, b);
        if (n != 3) continue;

        if (strcmp(kind, "backend") == 0 && cfg->n_be < MAX_BE) {
            backend_t *x = &cfg->be[cfg->n_be++];
            snprintf(x->name, sizeof(x->name), "%s", a);
            x->up = (strcmp(b, "up") == 0) ? 1 : 0;
            continue;
        }

        if (strcmp(kind, "route") == 0 && cfg->n_rt < MAX_RT) {
            route_t *r = &cfg->rt[cfg->n_rt++];
            snprintf(r->prefix, sizeof(r->prefix), "%s", a);
            snprintf(r->backend, sizeof(r->backend), "%s", b);
            continue;
        }
    }

    fclose(f);
    return 0;
}

static const backend_t *find_be(const cfg_t *cfg, const char *name) {
    for (size_t i = 0; i < cfg->n_be; ++i) {
        if (strcmp(cfg->be[i].name, name) == 0) return &cfg->be[i];
    }
    return NULL;
}

static const route_t *match_route(const cfg_t *cfg, const char *path) {
    const route_t *best = NULL;
    size_t best_len = 0;

    for (size_t i = 0; i < cfg->n_rt; ++i) {
        const route_t *r = &cfg->rt[i];
        size_t len = strlen(r->prefix);
        if (len == 0) continue;

        if (strncmp(path, r->prefix, len) == 0 && len > best_len) {
            best = r;
            best_len = len;
        }
    }

    return best;
}

int main(int argc, char **argv) {
    const char *cfg_path = "tests/data/gateway.sample";
    const char *path = "/api/v1/users";

    if (argc == 3) {
        cfg_path = argv[1];
        path = argv[2];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [<cfg> <path>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    cfg_t cfg;
    if (parse_cfg(cfg_path, &cfg) != 0) {
        perror("parse_cfg");
        return EXIT_FAILURE;
    }

    const route_t *r = match_route(&cfg, path);
    if (!r) {
        printf("path=%s plan=none reason=no_route\n", path);
        return EXIT_FAILURE;
    }

    const backend_t *be = find_be(&cfg, r->backend);
    if (!be || !be->up) {
        printf("path=%s plan=none reason=backend_down\n", path);
        return EXIT_FAILURE;
    }

    printf("path=%s plan=ok prefix=%s backend=%s\n", path, r->prefix, be->name);

    if (argc == 1) {
        return (strcmp(r->prefix, "/api/v1") == 0 && strcmp(be->name, "api_v1") == 0)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
