#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    /* Flag que cambia el modo de salida */
    int to_upper = 0;

    /*
     * Bucle de parseo de opciones cortas.
     * Aquí solo existe '-u'.
     */
    int opt;
    while ((opt = getopt(argc, argv, "u")) != -1) {
        switch (opt) {
            case 'u':
                to_upper = 1;
                break;
            default:
                fprintf(stderr, "Uso: %s [-u] [--] palabra...\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    /*
     * optind apunta al primer argumento posicional.
     * Si optind == argc no hay posicionales.
     */
    if (optind == argc) {
        fprintf(stderr, "Error: no se recibieron palabras\n");
        return EXIT_FAILURE;
    }

    for (int i = optind; i < argc; ++i) {
        const char *s = argv[i];

        /* Procesamos caracter por caracter para mostrar control de bytes */
        for (int j = 0; s[j] != '\0'; ++j) {
            unsigned char ch = (unsigned char)s[j];
            if (to_upper) {
                ch = (unsigned char)toupper(ch);
            }
            putchar((int)ch);
        }
        putchar('\n');
    }

    return EXIT_SUCCESS;
}
