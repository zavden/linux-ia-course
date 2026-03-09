#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Esta función imprime ayuda breve y directa.
 * Se acepta FILE* para poder enviarla a stdout (help normal)
 * o a stderr (error de uso).
 */
static void usage(FILE *out, const char *prog) {
    fprintf(out, "Uso: %s [-v] [-n NUM] [-h]\n", prog);
    fprintf(out, "  -v, --verbose      Activa salida detallada\n");
    fprintf(out, "  -n, --number NUM   Número entero >= 0\n");
    fprintf(out, "  -h, --help         Muestra ayuda\n");
}

int main(int argc, char **argv) {
    /* Defaults explícitos: importante para comportamiento determinista */
    int verbose = 0;
    int number = 0;

    /*
     * Tabla de opciones largas: mapea nombres largos a flags cortos.
     * El último elemento nulo es obligatorio.
     */
    static struct option long_opts[] = {
        {"verbose", no_argument, 0, 'v'},
        {"number", required_argument, 0, 'n'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "vn:h", long_opts, NULL)) != -1) {
        switch (opt) {
            case 'v':
                /* Flag booleana simple */
                verbose = 1;
                break;
            case 'n': {
                /*
                 * Parseo robusto de entero:
                 * - reiniciamos errno
                 * - validamos fin de string
                 * - validamos rango
                 */
                char *end = NULL;
                errno = 0;
                long value = strtol(optarg, &end, 10);
                if (errno != 0 || *end != '\0' || value < 0 || value > INT_MAX) {
                    fprintf(stderr, "Error: --number inválido: '%s'\n", optarg);
                    usage(stderr, argv[0]);
                    return EXIT_FAILURE;
                }
                number = (int)value;
                break;
            }
            case 'h':
                usage(stdout, argv[0]);
                return EXIT_SUCCESS;
            default:
                /* Caso opción desconocida o argumento faltante */
                usage(stderr, argv[0]);
                return EXIT_FAILURE;
        }
    }

    /* En este ejercicio no usamos posicionales; si llegan, se considera error */
    if (optind < argc) {
        fprintf(stderr, "Error: argumentos posicionales no soportados en E01\n");
        usage(stderr, argv[0]);
        return EXIT_FAILURE;
    }

    printf("verbose=%s\n", verbose ? "ON" : "OFF");
    printf("number=%d\n", number);
    return EXIT_SUCCESS;
}
