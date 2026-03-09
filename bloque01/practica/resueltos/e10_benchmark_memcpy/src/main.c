#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Implementación didáctica: simple, correcta, no optimizada */
static void *my_memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }

    return dest;
}

/* Utilidad de diferencia temporal en nanosegundos */
static long ns_diff(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1000000000L + (b.tv_nsec - a.tv_nsec);
}

int main(void) {
    const size_t n = 20 * 1024 * 1024;

    unsigned char *src = malloc(n);
    unsigned char *dst = malloc(n);
    if (!src || !dst) {
        perror("malloc");
        free(src);
        free(dst);
        return EXIT_FAILURE;
    }

    /* Inicializamos fuente para evitar páginas perezosas triviales */
    for (size_t i = 0; i < n; ++i) {
        src[i] = (unsigned char)(i & 0xFFU);
    }

    struct timespec t1, t2;

    clock_gettime(CLOCK_MONOTONIC, &t1);
    my_memcpy(dst, src, n);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    long my_ns = ns_diff(t1, t2);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    memcpy(dst, src, n);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    long libc_ns = ns_diff(t1, t2);

    printf("my_memcpy_ms=%.3f\n", my_ns / 1e6);
    printf("libc_memcpy_ms=%.3f\n", libc_ns / 1e6);

    free(src);
    free(dst);
    return EXIT_SUCCESS;
}
