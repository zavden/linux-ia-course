#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_blank_or_comment(const char *line) {
    size_t i = 0;
    while (line[i] != '\0' && isspace((unsigned char)line[i])) {
        i++;
    }
    return (line[i] == '\0' || line[i] == '#');
}

int main(int argc, char **argv) {
    /* Entrada por argumento para poder testear con fixtures reproducibles. */
    const char *path = (argc >= 2) ? argv[1] : "tests/data/acl.sample";

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen acl");
        return EXIT_FAILURE;
    }

    int users = 0;
    int groups = 0;
    int defaults = 0;

    char line[256];
    while (fgets(line, sizeof(line), f) != NULL) {
        /* Saltamos metadata/comentarios de getfacl. */
        if (is_blank_or_comment(line)) {
            continue;
        }

        /* ACL por defecto para nuevos hijos en directorios. */
        if (strncmp(line, "default:", 8) == 0) {
            defaults++;
            continue;
        }

        /* ACL normal de usuario (owner o nominal). */
        if (strncmp(line, "user:", 5) == 0) {
            users++;
            continue;
        }

        /* ACL normal de grupo (owner group o nominal). */
        if (strncmp(line, "group:", 6) == 0) {
            groups++;
            continue;
        }
    }

    fclose(f);

    /* Resumen compacto para ver distribución de entradas ACL. */
    printf("users=%d groups=%d defaults=%d\n", users, groups, defaults);

    return (users == 2 && groups == 2 && defaults == 3) ? EXIT_SUCCESS : EXIT_FAILURE;
}
