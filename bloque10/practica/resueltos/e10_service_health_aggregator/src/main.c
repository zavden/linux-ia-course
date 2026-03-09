#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int services;
    int ok;
    int warn;
    int crit;
} health_stats_t;

static char *trim(char *s) {
    while (*s != '\0' && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;

    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }
    return s;
}

static void lower_ascii(char *s) {
    for (size_t i = 0; s[i] != '\0'; ++i) {
        s[i] = (char)tolower((unsigned char)s[i]);
    }
}

static int parse_health(const char *path, health_stats_t *out) {
    if (path == NULL || out == NULL) return -1;

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    memset(out, 0, sizeof(*out));

    char line[512];
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
        char *service = trim(p);
        char *status = trim(eq + 1);
        (void)service; /* En este ejercicio no usamos nombre para reglas, solo conteo. */

        lower_ascii(status);

        out->services++;
        if (strcmp(status, "ok") == 0) {
            out->ok++;
        } else if (strcmp(status, "warn") == 0) {
            out->warn++;
        } else if (strcmp(status, "crit") == 0) {
            out->crit++;
        } else {
            /* Estado desconocido se trata como riesgo alto por postura defensiva. */
            out->crit++;
        }
    }

    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/health.warn.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<health_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    health_stats_t st;
    if (parse_health(path, &st) != 0) {
        perror("parse_health");
        return EXIT_FAILURE;
    }

    const char *final = "OK";
    if (st.crit > 0) {
        final = "CRIT";
    } else if (st.warn > 0) {
        final = "WARN";
    }

    printf("services=%d ok=%d warn=%d crit=%d final=%s\n",
           st.services, st.ok, st.warn, st.crit, final);

    if (argc == 1) {
        return (st.services == 6 && st.ok == 5 && st.warn == 1 && st.crit == 0 &&
                strcmp(final, "WARN") == 0)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
