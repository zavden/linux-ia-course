#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int found;
    int count;
    int best_pref;
    char best_host[256];
} mx_result_t;

/*
 * Normaliza nombre DNS a minusculas y remueve punto final opcional.
 * Asi evitamos diferencias cosmeticas al comparar dominios.
 */
static void normalize_dns_name(const char *in, char *out, size_t out_sz) {
    if (out_sz == 0U) return;

    size_t n = 0U;
    for (; in[n] != '\0' && n + 1U < out_sz; ++n) {
        out[n] = (char)tolower((unsigned char)in[n]);
    }
    out[n] = '\0';

    while (n > 0U && out[n - 1U] == '.') {
        out[--n] = '\0';
    }
}

static int parse_mx_file(const char *path, const char *zone, mx_result_t *out) {
    if (path == NULL || zone == NULL || out == NULL) return -1;

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    memset(out, 0, sizeof(*out));
    out->best_pref = 2147483647;

    char zone_norm[256];
    normalize_dns_name(zone, zone_norm, sizeof(zone_norm));

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';

        char *comment = strchr(line, ';');
        if (comment) {
            *comment = '\0';
        }

        char *p = line;
        while (*p != '\0' && isspace((unsigned char)*p)) p++;
        if (*p == '\0') continue;

        char *tokens[16];
        size_t ntok = 0U;

        char *tok = strtok(p, " \t");
        while (tok != NULL) {
            if (ntok >= (sizeof(tokens) / sizeof(tokens[0]))) {
                fclose(f);
                return -1;
            }
            tokens[ntok++] = tok;
            tok = strtok(NULL, " \t");
        }

        if (ntok < 6U) {
            continue;
        }

        if (strcmp(tokens[3], "MX") != 0) {
            continue;
        }

        char owner_norm[256];
        normalize_dns_name(tokens[0], owner_norm, sizeof(owner_norm));
        if (strcmp(owner_norm, zone_norm) != 0) {
            continue;
        }

        errno = 0;
        char *end = NULL;
        long pref = strtol(tokens[4], &end, 10);
        if (errno != 0 || end == tokens[4] || *end != '\0' || pref < 0 || pref > 65535) {
            continue;
        }

        char host_norm[256];
        normalize_dns_name(tokens[5], host_norm, sizeof(host_norm));

        out->count++;
        if (!out->found || pref < out->best_pref ||
            (pref == out->best_pref && strcmp(host_norm, out->best_host) < 0)) {
            out->found = 1;
            out->best_pref = (int)pref;
            snprintf(out->best_host, sizeof(out->best_host), "%s", host_norm);
        }
    }

    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    const char *path = "tests/data/mx.sample";
    const char *zone = "example.com.";

    if (argc == 3) {
        path = argv[1];
        zone = argv[2];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [<mx_file> <zone>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    mx_result_t r;
    if (parse_mx_file(path, zone, &r) != 0) {
        perror("parse_mx_file");
        return EXIT_FAILURE;
    }

    if (!r.found) {
        printf("zone=%s mx_count=0 best_pref=-1 best_host=none\n", zone);
        return EXIT_FAILURE;
    }

    printf("zone=%s mx_count=%d best_pref=%d best_host=%s\n",
           zone, r.count, r.best_pref, r.best_host);

    if (argc == 1) {
        return (r.count == 3 && r.best_pref == 10 && strcmp(r.best_host, "mail1.example.com") == 0)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
