#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * grow_buffer:
 * Realloc seguro usando temporal.
 */
static int grow_buffer(char **buf, size_t *cap, size_t new_cap) {
    /*
     * Nunca sobrescribimos *buf directo con realloc:
     * si falla, seguimos conservando el bloque viejo intacto.
     */
    char *tmp = realloc(*buf, new_cap);
    if (!tmp) {
        return -1;
    }

    /*
     * Inicializamos región nueva a '.' para visualizar crecimiento.
     * Esto no es obligatorio en producción, pero sí útil en aprendizaje.
     */
    if (new_cap > *cap) {
        memset(tmp + *cap, '.', new_cap - *cap);
    }

    *buf = tmp;
    *cap = new_cap;
    return 0;
}

int main(void) {
    /* Buffer inicial pequeño para observar crecimiento manual. */
    size_t cap = 8;
    char *buf = malloc(cap);
    if (!buf) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    /* Cargamos un prefijo conocido para validar integridad tras realloc. */
    memcpy(buf, "ABCDEF", 6);

    /* Realocamos a mayor tamaño usando helper seguro. */
    if (grow_buffer(&buf, &cap, 32) == -1) {
        perror("realloc");
        free(buf);
        return EXIT_FAILURE;
    }

    /* Debe conservar el prefijo original luego de la realocación. */
    printf("cap=%zu prefix=%.6s\n", cap, buf);

    /* Liberamos buffer final (sea o no movido por realloc). */
    free(buf);
    return EXIT_SUCCESS;
}
