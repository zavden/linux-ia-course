#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Reto C03:
 * Implementa minicat-lite con manejo real de edge cases.
 */

struct options {
    int number_all;
    int number_non_blank;
};

/*
 * TODO: parsear -n y -b.
 * Nota: -b tiene prioridad sobre -n en el comportamiento típico.
 */
static int parse_options(int argc, char **argv, struct options *opt, int *first_file_idx) {
    int i = 1;
    opt->number_all = 0;
    opt->number_non_blank = 0;

    while (i < argc && argv[i][0] == '-' && argv[i][1] != '\0') {
        if (strcmp(argv[i], "-") == 0) break; /* '-' es archivo stdin, no opción */
        if (strcmp(argv[i], "-n") == 0) {
            opt->number_all = 1;
        } else if (strcmp(argv[i], "-b") == 0) {
            opt->number_non_blank = 1;
        } else {
            fprintf(stderr, "Opción no soportada: %s\n", argv[i]);
            return -1;
        }
        i++;
    }

    *first_file_idx = i;
    return 0;
}

/*
 * TODO: implementar copia de stream con numeración opcional.
 * Pistas:
 * - usa fgets para leer líneas
 * - lleva contador de línea (long)
 * - aplica reglas de -n / -b
 */
static int cat_stream(FILE *in, const char *name, const struct options *opt) {
    (void)name;
    (void)opt;
    /* Placeholder intencional */
    char buf[1024];
    while (fgets(buf, sizeof(buf), in) != NULL) {
        fputs(buf, stdout);
    }
    if (ferror(in)) {
        fprintf(stderr, "Error leyendo stream\n");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    struct options opt;
    int first = 1;
    int had_error = 0;

    if (parse_options(argc, argv, &opt, &first) != 0) {
        return EXIT_FAILURE;
    }

    /*
     * Si no hay archivos, leer de stdin.
     * Si hay '-', también se interpreta como stdin.
     */
    if (first >= argc) {
        if (cat_stream(stdin, "stdin", &opt) != 0) had_error = 1;
    } else {
        for (int i = first; i < argc; ++i) {
            if (strcmp(argv[i], "-") == 0) {
                if (cat_stream(stdin, "stdin", &opt) != 0) had_error = 1;
                continue;
            }

            FILE *f = fopen(argv[i], "r");
            if (!f) {
                fprintf(stderr, "minicat-lite: %s: %s\n", argv[i], strerror(errno));
                had_error = 1;
                continue;
            }

            if (cat_stream(f, argv[i], &opt) != 0) had_error = 1;
            fclose(f);
        }
    }

    return had_error ? EXIT_FAILURE : EXIT_SUCCESS;
}
