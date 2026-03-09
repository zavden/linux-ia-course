#include <stdio.h>
#include <stdlib.h>

int main(void) {
    FILE *f = NULL;
    char *buffer = NULL;
    int rc = EXIT_FAILURE;

    /* Recurso #1: memoria dinámica */
    buffer = malloc(1024);
    if (!buffer) {
        perror("malloc");
        goto cleanup;
    }

    /* Recurso #2: archivo (forzamos error para demostrar el flujo) */
    f = fopen("/ruta/que/no/existe.txt", "r");
    if (!f) {
        perror("fopen");
        goto cleanup;
    }

    /* Si llegáramos aquí, todo habría salido bien */
    rc = EXIT_SUCCESS;

cleanup:
    /*
     * Este bloque se ejecuta SIEMPRE:
     * - en éxito
     * - en cualquier error temprano
     */
    if (f) {
        fclose(f);
    }

    free(buffer);

    return rc;
}
