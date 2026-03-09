#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *trim(char *s) {
    while (*s != '\0' && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
    return s;
}

static void lower_ascii(char *s) {
    for (size_t i = 0; s[i] != '\0'; ++i) s[i] = (char)tolower((unsigned char)s[i]);
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/readiness.warn.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<readiness_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    int total = 0, ok = 0, warn = 0, crit = 0;

    char line[256];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char *k = trim(line);
        char *v = trim(eq + 1);
        if (k[0] == '\0' || v[0] == '\0') continue;

        lower_ascii(v);

        total++;
        if (strcmp(v, "ok") == 0) ok++;
        else if (strcmp(v, "warn") == 0) warn++;
        else crit++;
    }

    fclose(f);

    const char *status = "OK";
    int ready = 1;

    if (crit > 0) {
        status = "CRIT";
        ready = 0;
    } else if (warn > 0) {
        status = "WARN";
        ready = 0;
    }

    printf("total=%d ok=%d warn=%d crit=%d status=%s ready=%d\n",
           total, ok, warn, crit, status, ready);

    return ready ? EXIT_SUCCESS : EXIT_FAILURE;
}
