#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4
#define N 1000

typedef struct {
    /* Identificador lógico de hilo (solo para trazas). */
    int tid;
    /* Índice inicial del segmento asignado (incluyente). */
    int start;
    /* Índice final del segmento (excluyente). */
    int end;
    /* Arreglo compartido, solo lectura para todos los hilos. */
    const int *data;
    /* Resultado parcial escrito por ESTE hilo. */
    long partial;
} worker_args_t;

/*
 * Cada hilo suma un rango [start, end) del arreglo compartido.
 * No requiere mutex porque cada hilo escribe solo su campo partial.
 */
static void *worker_sum(void *arg) {
    worker_args_t *w = (worker_args_t *)arg;
    long acc = 0;

    for (int i = w->start; i < w->end; ++i) {
        acc += w->data[i];
    }

    w->partial = acc;
    printf("thread=%d range=[%d,%d) partial=%ld\n", w->tid, w->start, w->end, w->partial);
    return NULL;
}

int main(void) {
    int data[N];

    /* Inicializamos 1..N para tener suma conocida (N*(N+1)/2). */
    for (int i = 0; i < N; ++i) {
        data[i] = i + 1;
    }

    pthread_t th[NUM_THREADS];
    worker_args_t args[NUM_THREADS];

    /* Reparto de carga homogéneo por bloques contiguos. */
    int chunk = N / NUM_THREADS;
    for (int t = 0; t < NUM_THREADS; ++t) {
        args[t].tid = t;
        args[t].start = t * chunk;
        args[t].end = (t == NUM_THREADS - 1) ? N : (t + 1) * chunk;
        args[t].data = data;
        args[t].partial = 0;

        /* Cada hilo recibe su propia estructura de argumentos. */
        if (pthread_create(&th[t], NULL, worker_sum, &args[t]) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    long total = 0;
    for (int t = 0; t < NUM_THREADS; ++t) {
        /* join garantiza que partial ya fue escrito cuando lo leamos. */
        if (pthread_join(th[t], NULL) != 0) {
            perror("pthread_join");
            return EXIT_FAILURE;
        }
        /* Acumulamos de forma secuencial en el hilo principal. */
        total += args[t].partial;
    }

    long expected = (long)N * (N + 1) / 2;
    printf("threads_joined=%d total=%ld expected=%ld\n", NUM_THREADS, total, expected);

    return (total == expected) ? EXIT_SUCCESS : EXIT_FAILURE;
}
