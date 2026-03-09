#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void chomp(char *s) {
    s[strcspn(s, "\r\n")] = '\0';
}

static int valid_status(const char *s) {
    return (strcmp(s, "up") == 0 || strcmp(s, "down") == 0) ? 1 : 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/registry.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<registry_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    int total = 0;
    int up = 0;
    int down = 0;
    int invalid = 0;

    char line[512];
    while (fgets(line, sizeof(line), f) != NULL) {
        chomp(line);
        if (line[0] == '\0' || line[0] == '#') continue;

        char copy[512];
        snprintf(copy, sizeof(copy), "%s", line);

        char *fields[4] = {0};
        int nf = 0;

        char *save = NULL;
        char *tok = strtok_r(copy, "|", &save);
        while (tok != NULL && nf < 4) {
            fields[nf++] = tok;
            tok = strtok_r(NULL, "|", &save);
        }

        if (nf != 4 || tok != NULL) {
            invalid++;
            continue;
        }

        if (!valid_status(fields[3])) {
            invalid++;
            continue;
        }

        total++;
        if (strcmp(fields[3], "up") == 0) up++;
        else down++;
    }

    fclose(f);

    printf("{\"total\":%d,\"up\":%d,\"down\":%d,\"invalid\":%d}\n", total, up, down, invalid);

    if (argc == 1) {
        return (total == 5 && up == 4 && down == 1 && invalid == 1) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
