#define _POSIX_C_SOURCE 200809L
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Riesgo clasico:
 * strcmp() corta en el primer byte distinto.
 * Eso puede revelar prefijos correctos midiendo tiempo de respuesta.
 *
 * Idea de mitigacion:
 * recorrer siempre la longitud maxima y acumular diferencias.
 */

static int constant_time_eq(const char *a, const char *b) {
    size_t alen = strlen(a);
    size_t blen = strlen(b);
    size_t n = (alen > blen) ? alen : blen;

    /* Incluimos diferencia de longitudes en el acumulador. */
    unsigned diff = (unsigned)(alen ^ blen);

    for (size_t i = 0; i < n; ++i) {
        unsigned char ca = (i < alen) ? (unsigned char)a[i] : 0U;
        unsigned char cb = (i < blen) ? (unsigned char)b[i] : 0U;
        diff |= (unsigned)(ca ^ cb);
    }

    return (diff == 0U) ? 1 : 0;
}

int main(int argc, char **argv) {
    const char *expected = "A1B2C3D4E5F60708";
    const char *candidate = expected;

    if (argc == 2) {
        candidate = argv[1];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [<token>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int len_ok = (strlen(candidate) == strlen(expected)) ? 1 : 0;

    /* Comparacion robusta para secretos. */
    int ct_match = constant_time_eq(candidate, expected);

    /* Comparacion ingenua incluida para contraste didactico. */
    int naive_match = (strcmp(candidate, expected) == 0) ? 1 : 0;

    printf("len_ok=%d ct_match=%d naive_match=%d\n", len_ok, ct_match, naive_match);

    return (ct_match == 1) ? EXIT_SUCCESS : EXIT_FAILURE;
}
