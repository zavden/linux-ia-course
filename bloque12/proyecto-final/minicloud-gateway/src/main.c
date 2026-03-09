#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BACKENDS 64
#define MAX_ROUTES 128

typedef struct {
    char name[64];
    int up;
} backend_t;

typedef struct {
    char prefix[128];
    char backend[64];
} route_t;

typedef struct {
    backend_t backends[MAX_BACKENDS];
    size_t n_backends;
    route_t routes[MAX_ROUTES];
    size_t n_routes;
} gateway_cfg_t;

static int parse_cfg(const char *path, gateway_cfg_t *cfg) {
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

        if (strcmp(kind, "backend") == 0 && cfg->n_backends < MAX_BACKENDS) {
            backend_t *be = &cfg->backends[cfg->n_backends++];
            snprintf(be->name, sizeof(be->name), "%s", a);
            be->up = (strcmp(b, "up") == 0) ? 1 : 0;
            continue;
        }

        if (strcmp(kind, "route") == 0 && cfg->n_routes < MAX_ROUTES) {
            route_t *rt = &cfg->routes[cfg->n_routes++];
            snprintf(rt->prefix, sizeof(rt->prefix), "%s", a);
            snprintf(rt->backend, sizeof(rt->backend), "%s", b);
            continue;
        }
    }

    fclose(f);
    return 0;
}

static const backend_t *find_backend(const gateway_cfg_t *cfg, const char *name) {
    for (size_t i = 0; i < cfg->n_backends; ++i) {
        if (strcmp(cfg->backends[i].name, name) == 0) return &cfg->backends[i];
    }
    return NULL;
}

/*
 * Longest-prefix-match: selecciona la regla mas especifica.
 */
static const route_t *match_route(const gateway_cfg_t *cfg, const char *path) {
    const route_t *best = NULL;
    size_t best_len = 0U;

    for (size_t i = 0; i < cfg->n_routes; ++i) {
        const route_t *r = &cfg->routes[i];
        size_t len = strlen(r->prefix);

        if (len == 0U) continue;

        if (strncmp(path, r->prefix, len) == 0 && len > best_len) {
            best = r;
            best_len = len;
        }
    }

    return best;
}

int main(int argc, char **argv) {
    const char *cfg_path = "tests/data/routes.sample";
    const char *path = "/api/v1/users";

    if (argc == 3) {
        cfg_path = argv[1];
        path = argv[2];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [<routes_file> <path>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    gateway_cfg_t cfg;
    if (parse_cfg(cfg_path, &cfg) != 0) {
        perror("parse_cfg");
        return EXIT_FAILURE;
    }

    const route_t *r = match_route(&cfg, path);
    if (!r) {
        printf("path=%s plan=none reason=no_route\n", path);
        return EXIT_FAILURE;
    }

    const backend_t *be = find_backend(&cfg, r->backend);
    if (!be || !be->up) {
        printf("path=%s plan=none reason=backend_down\n", path);
        return EXIT_FAILURE;
    }

    printf("path=%s plan=ok prefix=%s backend=%s\n", path, r->prefix, be->name);
    return EXIT_SUCCESS;
}
