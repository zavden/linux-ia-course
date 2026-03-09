#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    const char *name;
    int64_t elapsed_ns;
} scope_metric_t;

static int64_t now_ns(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return -1;
    }
    return (int64_t)ts.tv_sec * 1000000000LL + (int64_t)ts.tv_nsec;
}

/* Carga de CPU corta para simular trabajo de una etapa. */
static void workload_a(void) {
    volatile uint64_t x = 0;
    for (uint64_t i = 0; i < 800000ULL; ++i) {
        x += (i * 3ULL) % 101ULL;
    }
}

/* Segunda etapa independiente para perfilar por separado. */
static void workload_b(void) {
    volatile uint64_t x = 1;
    for (uint64_t i = 1; i < 600000ULL; ++i) {
        x ^= (i * 7ULL);
    }
}

/* Mide una función y guarda resultado en una métrica. */
static int profile_scope(scope_metric_t *m, void (*fn)(void)) {
    int64_t t0 = now_ns();
    if (t0 < 0) {
        return -1;
    }

    fn();

    int64_t t1 = now_ns();
    if (t1 < 0) {
        return -1;
    }

    m->elapsed_ns = t1 - t0;
    return 0;
}

int main(void) {
    scope_metric_t scopes[2] = {
        {.name = "load_A", .elapsed_ns = 0},
        {.name = "load_B", .elapsed_ns = 0},
    };

    /* Perfilamos cada etapa por separado para aislar costo relativo. */
    if (profile_scope(&scopes[0], workload_a) == -1 ||
        profile_scope(&scopes[1], workload_b) == -1) {
        perror("profile_scope");
        return EXIT_FAILURE;
    }

    /* Métrica agregada simple (suma de scopes). */
    int64_t total = scopes[0].elapsed_ns + scopes[1].elapsed_ns;

    /* Reporte legible para análisis rápido y tests. */
    printf("%s=%lld %s=%lld total=%lld\n",
           scopes[0].name,
           (long long)scopes[0].elapsed_ns,
           scopes[1].name,
           (long long)scopes[1].elapsed_ns,
           (long long)total);

    return (scopes[0].elapsed_ns > 0 && scopes[1].elapsed_ns > 0 && total > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
