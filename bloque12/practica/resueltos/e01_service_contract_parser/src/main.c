#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Contrato esperado por linea:
 * service|port|proto|health_path
 * Ejemplo:
 * registry|8500|tcp|/health
 */

static int is_valid_service_char(unsigned char c) {
    return (isalnum(c) != 0) || c == '-' || c == '_';
}

static int valid_service_name(const char *s) {
    if (s == NULL) return 0;

    size_t len = strlen(s);
    if (len < 3U || len > 32U) return 0;

    for (size_t i = 0; i < len; ++i) {
        if (!is_valid_service_char((unsigned char)s[i])) return 0;
    }

    return 1;
}

static int valid_port(const char *s, unsigned *out) {
    if (s == NULL || out == NULL || s[0] == '\0') return 0;

    if (s[0] == '+' || s[0] == '-') return 0;

    errno = 0;
    char *end = NULL;
    unsigned long p = strtoul(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || p == 0UL || p > 65535UL) {
        return 0;
    }

    *out = (unsigned)p;
    return 1;
}

static int valid_proto(const char *s) {
    return (strcmp(s, "tcp") == 0 || strcmp(s, "http") == 0 || strcmp(s, "https") == 0) ? 1 : 0;
}

static int valid_health_path(const char *s) {
    if (s == NULL || s[0] != '/') return 0;
    if (strlen(s) > 128U) return 0;
    return 1;
}

static void chomp(char *s) {
    if (!s) return;
    s[strcspn(s, "\r\n")] = '\0';
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/contracts.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<contracts_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    int valid = 0;
    int invalid = 0;
    int https = 0;

    char line[512];
    while (fgets(line, sizeof(line), f) != NULL) {
        chomp(line);

        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }

        char copy[512];
        if (snprintf(copy, sizeof(copy), "%s", line) >= (int)sizeof(copy)) {
            invalid++;
            continue;
        }

        char *fields[4] = {0};
        int nf = 0;

        char *save = NULL;
        char *tok = strtok_r(copy, "|", &save);
        while (tok != NULL && nf < 4) {
            fields[nf++] = tok;
            tok = strtok_r(NULL, "|", &save);
        }

        /* Si faltan o sobran campos, contrato invalido. */
        if (nf != 4 || tok != NULL) {
            invalid++;
            continue;
        }

        unsigned port = 0U;
        int ok = valid_service_name(fields[0]) &&
                 valid_port(fields[1], &port) &&
                 valid_proto(fields[2]) &&
                 valid_health_path(fields[3]);

        if (!ok) {
            invalid++;
            continue;
        }

        valid++;
        if (strcmp(fields[2], "https") == 0) {
            https++;
        }

        (void)port;
    }

    fclose(f);

    printf("valid=%d invalid=%d https=%d\n", valid, invalid, https);

    if (argc == 1) {
        return (valid == 4 && invalid == 1 && https == 2) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
