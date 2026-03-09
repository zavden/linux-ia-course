#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BACKENDS 32
#define MAX_ROUTES 64

typedef struct {
    char name[64];
    char target[128];
} backend_t;

typedef struct {
    char prefix[128];
    char backend_name[64];
} route_t;

typedef struct {
    backend_t backends[MAX_BACKENDS];
    size_t n_backends;
    route_t routes[MAX_ROUTES];
    size_t n_routes;
} proxy_cfg_t;

static int parse_cfg(const char *path, proxy_cfg_t *cfg) {
    if (path == NULL || cfg == NULL) return -1;

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    memset(cfg, 0, sizeof(*cfg));

    char line[512];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';

        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }

        char kind[32];
        char a[128];
        char b[128];

        int n = sscanf(line, "%31s %127s %127s", kind, a, b);
        if (n != 3) {
            continue;
        }

        if (strcmp(kind, "backend") == 0) {
            if (cfg->n_backends >= MAX_BACKENDS) {
                fclose(f);
                return -1;
            }

            backend_t *be = &cfg->backends[cfg->n_backends++];
            snprintf(be->name, sizeof(be->name), "%s", a);
            snprintf(be->target, sizeof(be->target), "%s", b);
            continue;
        }

        if (strcmp(kind, "route") == 0) {
            if (cfg->n_routes >= MAX_ROUTES) {
                fclose(f);
                return -1;
            }

            route_t *rt = &cfg->routes[cfg->n_routes++];
            snprintf(rt->prefix, sizeof(rt->prefix), "%s", a);
            snprintf(rt->backend_name, sizeof(rt->backend_name), "%s", b);
            continue;
        }
    }

    fclose(f);
    return 0;
}

static const backend_t *find_backend(const proxy_cfg_t *cfg, const char *name) {
    for (size_t i = 0; i < cfg->n_backends; ++i) {
        if (strcmp(cfg->backends[i].name, name) == 0) {
            return &cfg->backends[i];
        }
    }
    return NULL;
}

/*
 * Longest prefix match: la ruta mas especifica gana.
 * Ej: /api/v1 > /api > /
 */
static const route_t *match_route(const proxy_cfg_t *cfg, const char *path) {
    const route_t *best = NULL;
    size_t best_len = 0U;

    for (size_t i = 0; i < cfg->n_routes; ++i) {
        const route_t *r = &cfg->routes[i];
        size_t len = strlen(r->prefix);

        if (len == 0U) {
            continue;
        }

        if (strncmp(path, r->prefix, len) == 0) {
            if (len > best_len) {
                best = r;
                best_len = len;
            }
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
        fprintf(stderr, "Uso: %s [<proxy_cfg> <path>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    proxy_cfg_t cfg;
    if (parse_cfg(cfg_path, &cfg) != 0) {
        perror("parse_cfg");
        return EXIT_FAILURE;
    }

    const route_t *r = match_route(&cfg, path);
    if (r == NULL) {
        printf("path=%s matched=0\n", path);
        return EXIT_FAILURE;
    }

    const backend_t *be = find_backend(&cfg, r->backend_name);
    if (be == NULL) {
        printf("path=%s matched=0 reason=backend_missing\n", path);
        return EXIT_FAILURE;
    }

    printf("path=%s matched=1 prefix=%s backend=%s target=%s\n",
           path, r->prefix, be->name, be->target);

    if (argc == 1) {
        return (strcmp(be->name, "api") == 0 && strcmp(r->prefix, "/api") == 0)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
