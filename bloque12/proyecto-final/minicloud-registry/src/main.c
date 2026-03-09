#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Registro esperado:
 * service|host|port|proto|health_path|status
 */

typedef struct {
    int valid;
    int invalid;
    int up;
    int down;
    int secure;
} registry_stats_t;

static int valid_name(const char *s) {
    if (!s || s[0] == '\0') return 0;
    size_t len = strlen(s);
    if (len < 3U || len > 32U) return 0;

    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '-' || c == '_')) return 0;
    }

    return 1;
}

static int valid_host(const char *s) {
    if (!s || s[0] == '\0') return 0;
    size_t len = strlen(s);
    if (len < 3U || len > 253U) return 0;

    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '-' || c == '.' || c == '_')) return 0;
    }

    return 1;
}

static int valid_port(const char *s, unsigned *out) {
    if (!s || !out || s[0] == '\0') return 0;
    if (s[0] == '+' || s[0] == '-') return 0;

    errno = 0;
    char *end = NULL;
    unsigned long p = strtoul(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || p == 0UL || p > 65535UL) return 0;

    *out = (unsigned)p;
    return 1;
}

static int valid_proto(const char *s) {
    return (strcmp(s, "tcp") == 0 || strcmp(s, "http") == 0 || strcmp(s, "https") == 0) ? 1 : 0;
}

static int valid_path(const char *s) {
    if (!s || s[0] != '/') return 0;
    return (strlen(s) <= 128U) ? 1 : 0;
}

static int valid_status(const char *s) {
    return (strcmp(s, "up") == 0 || strcmp(s, "down") == 0) ? 1 : 0;
}

static int parse_registry(const char *path, registry_stats_t *st) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    memset(st, 0, sizeof(*st));

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        char copy[1024];
        snprintf(copy, sizeof(copy), "%s", line);

        char *fields[6] = {0};
        int nf = 0;

        char *save = NULL;
        char *tok = strtok_r(copy, "|", &save);
        while (tok && nf < 6) {
            fields[nf++] = tok;
            tok = strtok_r(NULL, "|", &save);
        }

        if (nf != 6 || tok != NULL) {
            st->invalid++;
            continue;
        }

        unsigned port = 0;
        int ok = valid_name(fields[0]) &&
                 valid_host(fields[1]) &&
                 valid_port(fields[2], &port) &&
                 valid_proto(fields[3]) &&
                 valid_path(fields[4]) &&
                 valid_status(fields[5]);

        if (!ok) {
            st->invalid++;
            continue;
        }

        st->valid++;
        if (strcmp(fields[5], "up") == 0) st->up++;
        else st->down++;

        if (strcmp(fields[3], "https") == 0) {
            st->secure++;
        }

        (void)port;
    }

    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/contracts.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<contracts_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    registry_stats_t st;
    if (parse_registry(path, &st) != 0) {
        perror("parse_registry");
        return EXIT_FAILURE;
    }

    int ready = (st.invalid == 0 && st.up >= 3) ? 1 : 0;

    printf("valid=%d invalid=%d up=%d down=%d secure=%d ready=%d\n",
           st.valid, st.invalid, st.up, st.down, st.secure, ready);

    return ready ? EXIT_SUCCESS : EXIT_FAILURE;
}
