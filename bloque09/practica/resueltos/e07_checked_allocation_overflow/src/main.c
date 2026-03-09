#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Vulnerabilidad clasica:
 * bytes = n * elem_size;
 * si overflowea, bytes queda pequeno y luego se escribe de mas.
 *
 * Este ejercicio implementa la forma segura de calcular ese producto.
 */

/* Conversion estricta hacia size_t (sin signos ni basura final). */
static int parse_size_strict(const char *text, size_t *out) {
    if (text == NULL || out == NULL || text[0] == '\0') {
        return -1;
    }

    if (text[0] == '+' || text[0] == '-') {
        return -1;
    }

    errno = 0;
    char *end = NULL;
    unsigned long long value = strtoull(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0') {
        return -1;
    }

    if (value > (unsigned long long)SIZE_MAX) {
        return -1;
    }

    *out = (size_t)value;
    return 0;
}

/*
 * Multiplicacion protegida:
 * a * b es segura solo si a <= SIZE_MAX / b (cuando b != 0).
 */
static int checked_mul_size(size_t a, size_t b, size_t *out) {
    if (out == NULL) {
        return -1;
    }

    if (a == 0U || b == 0U) {
        *out = 0U;
        return 0;
    }

    if (a > SIZE_MAX / b) {
        return -1;
    }

    *out = a * b;
    return 0;
}

int main(int argc, char **argv) {
    /* Valores por defecto moderados para pruebas rapidas. */
    size_t n = 128U;
    size_t elem_size = 64U;

    if (argc == 3) {
        if (parse_size_strict(argv[1], &n) != 0 || parse_size_strict(argv[2], &elem_size) != 0) {
            fprintf(stderr, "Parametros invalidos\n");
            return EXIT_FAILURE;
        }
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [<n> <elem_size>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t bytes = 0U;
    if (checked_mul_size(n, elem_size, &bytes) != 0) {
        /* Reporte explicito de overflow para monitoreo y test. */
        printf("n=%zu elem=%zu overflow=1\n", n, elem_size);
        return EXIT_FAILURE;
    }

    /*
     * calloc evita basura inicial y facilita ejemplo pedagogico.
     * Si bytes == 0, calloc puede devolver NULL o puntero valido; ambos son aceptables.
     */
    unsigned char *buffer = (unsigned char *)calloc(1U, bytes);
    if (bytes != 0U && buffer == NULL) {
        perror("calloc");
        return EXIT_FAILURE;
    }

    /* Patrón deterministico para demostrar que el buffer existe y es escribible. */
    unsigned checksum = 0U;
    for (size_t i = 0; i < bytes; ++i) {
        buffer[i] = (unsigned char)(i & 0xFFU);
        checksum = (checksum + (unsigned)buffer[i]) & 0xFFFFU;
    }

    printf("n=%zu elem=%zu bytes=%zu overflow=0 checksum=%u\n", n, elem_size, bytes, checksum);

    free(buffer);
    return EXIT_SUCCESS;
}
