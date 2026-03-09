#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Reto C01:
 * Implementa una CLI robusta de verdad.
 * Este archivo tiene guía en comentarios, pero no la solución final.
 */

/*
 * Sugerencia: encapsula toda la configuración parseada en una struct,
 * así evitas variables sueltas y facilitas pruebas.
 */
struct config {
    int verbose;
    int dry_run;
    int number;
    const char *output;
};

static void usage(FILE *out, const char *prog) {
    fprintf(out, "Uso: %s [-v] [--dry-run] -o FILE -n NUM [--] args...\n", prog);
}

int main(int argc, char **argv) {
    /* TODO 1: inicializa defaults razonables */
    struct config cfg = {0, 0, 0, "out.txt"};

    /* TODO 2: define tabla long_opts */
    static struct option long_opts[] = {
        {"verbose", no_argument, 0, 'v'},
        {"output", required_argument, 0, 'o'},
        {"number", required_argument, 0, 'n'},
        {"dry-run", no_argument, 0, 1000},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    /*
     * TODO 3: parsea con getopt_long.
     * - valida -n con strtol + errno + rango.
     * - maneja -h mostrando usage y saliendo 0.
     * - cualquier error de uso debe salir no-cero.
     */
    int c;
    while ((c = getopt_long(argc, argv, "vo:n:h", long_opts, NULL)) != -1) {
        switch (c) {
            case 'v':
                cfg.verbose = 1;
                break;
            case 'o':
                cfg.output = optarg;
                break;
            case 'n': {
                /* TODO: reemplaza este parseo por uno robusto */
                char *end = NULL;
                errno = 0;
                long n = strtol(optarg, &end, 10);
                if (errno != 0 || *end != '\0' || n < 0 || n > INT_MAX) {
                    fprintf(stderr, "number inválido: %s\n", optarg);
                    usage(stderr, argv[0]);
                    return EXIT_FAILURE;
                }
                cfg.number = (int)n;
                break;
            }
            case 'h':
                usage(stdout, argv[0]);
                return EXIT_SUCCESS;
            case 1000:
                cfg.dry_run = 1;
                break;
            default:
                usage(stderr, argv[0]);
                return EXIT_FAILURE;
        }
    }

    /*
     * TODO 4:
     * - procesa posicionales desde argv[optind]
     * - define si son obligatorios o opcionales
     * - documenta esa decisión en README
     */

    printf("verbose=%d dry_run=%d number=%d output=%s\n",
           cfg.verbose, cfg.dry_run, cfg.number, cfg.output);

    /* TODO 5: imprime también los posicionales de forma clara */

    return EXIT_SUCCESS;
}
