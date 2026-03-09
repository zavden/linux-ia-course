#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int exports_count;
    int hosts_total;
    int rw_hosts;
    int ro_hosts;
    int no_root_squash;
} nfs_stats_t;

static char *trim(char *s) {
    while (*s != '\0' && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;

    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }
    return s;
}

/*
 * Busca opcion exacta sin usar strtok global, para no interferir
 * con el parseo principal de la linea de exports.
 */
static int has_option(const char *opts, const char *needle) {
    size_t nlen = strlen(needle);
    const char *p = opts;

    while (*p != '\0') {
        while (*p == ',' || isspace((unsigned char)*p)) {
            p++;
        }

        if (*p == '\0') {
            break;
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
        if (len == nlen && strncmp(start, needle, nlen) == 0) {
            return 1;
        }
    }

    return 0;
}

static int parse_exports(const char *path, nfs_stats_t *out) {
    if (path == NULL || out == NULL) return -1;

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    memset(out, 0, sizeof(*out));

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

        /* Token 0: path exportado. El resto: clientes(opciones). */
        char *tok = strtok(p, " \t");
        if (tok == NULL) {
            continue;
        }

        out->exports_count++;

        tok = strtok(NULL, " \t");
        while (tok != NULL) {
            char *lp = strchr(tok, '(');
            char *rp = (lp != NULL) ? strrchr(tok, ')') : NULL;

            if (lp != NULL && rp != NULL && rp > lp + 1) {
                *rp = '\0';
                const char *opts = lp + 1;

                out->hosts_total++;

                if (has_option(opts, "rw")) {
                    out->rw_hosts++;
                }
                if (has_option(opts, "ro")) {
                    out->ro_hosts++;
                }
                if (has_option(opts, "no_root_squash")) {
                    out->no_root_squash++;
                }
            }

            tok = strtok(NULL, " \t");
        }
    }

    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/exports.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<exports_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    nfs_stats_t st;
    if (parse_exports(path, &st) != 0) {
        perror("parse_exports");
        return EXIT_FAILURE;
    }

    printf("exports=%d hosts=%d rw_hosts=%d ro_hosts=%d no_root_squash=%d\n",
           st.exports_count, st.hosts_total, st.rw_hosts, st.ro_hosts, st.no_root_squash);

    if (argc == 1) {
        return (st.exports_count == 2 && st.hosts_total == 3 && st.rw_hosts == 2 &&
                st.ro_hosts == 1 && st.no_root_squash == 1)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
