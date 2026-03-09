#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int valid_name(const char *s) {
    if (!s || s[0] == '\0') return 0;
    size_t len = strlen(s);
    if (len < 3U || len > 40U) return 0;

    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '-' || c == '_' || c == '.')) return 0;
    }
    return 1;
}

static int parse_int_range(const char *s, long minv, long maxv, long *out) {
    if (!s || !out || s[0] == '\0') return 0;
    if (s[0] == '+' || s[0] == '-') return 0;

    errno = 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return 0;
    if (v < minv || v > maxv) return 0;

    *out = v;
    return 1;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/jobs.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<jobs_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    int valid = 0;
    int invalid = 0;
    int heavy = 0;

    char line[512];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        char copy[512];
        snprintf(copy, sizeof(copy), "%s", line);

        char *f1 = strtok(copy, "|");
        char *f2 = strtok(NULL, "|");
        char *f3 = strtok(NULL, "|");
        char *f4 = strtok(NULL, "|");
        char *f5 = strtok(NULL, "|");
        char *extra = strtok(NULL, "|");

        if (!f1 || !f2 || !f3 || !f4 || !f5 || extra) {
            invalid++;
            continue;
        }

        long cpu = 0, mem = 0, to = 0;
        int ok = valid_name(f1) && valid_name(f2) &&
                 parse_int_range(f3, 50, 4000, &cpu) &&
                 parse_int_range(f4, 64, 16384, &mem) &&
                 parse_int_range(f5, 5, 3600, &to);

        if (!ok) {
            invalid++;
            continue;
        }

        valid++;
        if (cpu >= 2000 || mem >= 4096) {
            heavy++;
        }

        (void)to;
    }

    fclose(f);

    printf("valid=%d invalid=%d heavy=%d\n", valid, invalid, heavy);

    if (argc == 1) {
        return (valid == 4 && invalid == 1 && heavy == 2) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
