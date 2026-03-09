#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Convierte timespec a nanosegundos enteros. */
static int64_t to_ns(struct timespec ts) {
    return (int64_t)ts.tv_sec * 1000000000LL + (int64_t)ts.tv_nsec;
}

int main(void) {
    struct timespec t0;
    struct timespec t1;

    /* Timestamp inicial del bloque a medir. */
    if (clock_gettime(CLOCK_MONOTONIC, &t0) != 0) {
        perror("clock_gettime t0");
        return EXIT_FAILURE;
    }

    /*
     * Carga sintética simple para benchmark.
     * volatile evita que el compilador elimine el loop por optimización.
     */
    volatile uint64_t sink = 0;
    for (uint64_t i = 0; i < 2000000ULL; ++i) {
        sink += (i % 97ULL);
    }

    /* Timestamp final del mismo bloque. */
    if (clock_gettime(CLOCK_MONOTONIC, &t1) != 0) {
        perror("clock_gettime t1");
        return EXIT_FAILURE;
    }

    /* elapsed_ns mide solo la región de trabajo entre t0 y t1. */
    int64_t elapsed = to_ns(t1) - to_ns(t0);
    /* sink evita optimización agresiva que eliminaría el bucle. */
    printf("elapsed_ns=%lld sink=%llu\n", (long long)elapsed, (unsigned long long)sink);

    return (elapsed > 0 && sink > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
