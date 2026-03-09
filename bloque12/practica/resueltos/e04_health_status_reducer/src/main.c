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
    const char *path = (argc == 2) ? argv[1] : "tests/data/health.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<health_file>]\n", argv[0]);
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
        char *svc = trim(line);
        char *st = trim(eq + 1);
        if (svc[0] == '\0' || st[0] == '\0') continue;

        lower_ascii(st);

        total++;
        if (strcmp(st, "ok") == 0) ok++;
        else if (strcmp(st, "warn") == 0) warn++;
        else crit++; /* Unknown => conservative critical */
    }

    fclose(f);

    const char *global = "OK";
    if (crit > 0) global = "CRIT";
    else if (warn > 0) global = "WARN";

    printf("total=%d ok=%d warn=%d crit=%d global=%s\n", total, ok, warn, crit, global);

    if (argc == 1) {
        return (total == 5 && ok == 3 && warn == 1 && crit == 1 && strcmp(global, "CRIT") == 0)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
