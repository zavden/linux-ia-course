#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int valid_name(const char *s) {
    if (!s || s[0] == '\0') return 0;
    size_t len = strlen(s);
    if (len < 3U || len > 48U) return 0;

    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '_' || c == '-' || c == '.')) return 0;
    }

    return 1;
}

static int valid_scope(const char *s) {
    return (strcmp(s, "global") == 0 || strcmp(s, "service") == 0 || strcmp(s, "tenant") == 0) ? 1 : 0;
}

static int parse_u64(const char *s, unsigned long long *out) {
    if (!s || !out || s[0] == '\0') return 0;
    if (s[0] == '+' || s[0] == '-') return 0;

    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return 0;

    *out = v;
    return 1;
}

static int valid_rot(const char *s, int *out_rot) {
    if (!s || !out_rot) return 0;
    if (strcmp(s, "yes") == 0) {
        *out_rot = 1;
        return 1;
    }
    if (strcmp(s, "no") == 0) {
        *out_rot = 0;
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/secrets.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<manifest_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    int valid = 0;
    int invalid = 0;
    int global = 0;
    int rotatable = 0;

    char line[512];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        char copy[512];
        snprintf(copy, sizeof(copy), "%s", line);

        char *fields[4] = {0};
        int nf = 0;
        char *save = NULL;
        char *tok = strtok_r(copy, "|", &save);
        while (tok && nf < 4) {
            fields[nf++] = tok;
            tok = strtok_r(NULL, "|", &save);
        }

        if (nf != 4 || tok != NULL) {
            invalid++;
            continue;
        }

        unsigned long long ttl = 0ULL;
        int rot = 0;

        int ok = valid_name(fields[0]) &&
                 valid_scope(fields[1]) &&
                 parse_u64(fields[2], &ttl) &&
                 valid_rot(fields[3], &rot) &&
                 ttl >= 60ULL && ttl <= 31536000ULL;

        if (!ok) {
            invalid++;
            continue;
        }

        valid++;
        if (strcmp(fields[1], "global") == 0) global++;
        if (rot) rotatable++;
    }

    fclose(f);

    printf("valid=%d invalid=%d global=%d rotatable=%d\n", valid, invalid, global, rotatable);

    if (argc == 1) {
        return (valid == 4 && invalid == 1 && global == 2 && rotatable == 3)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
