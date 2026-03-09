#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    /* Identificador para logging y depuración. */
    int tid;
    /* Rango de trabajo inclusivo [from..to]. */
    int from;
    int to;
} job_t;

/*
 * Demostramos patrón clásico:
 * 1) argumentos complejos por struct
 * 2) valor de retorno por puntero heap recuperado en pthread_join
 */
static void *worker_sum_sq(void *arg) {
    job_t *j = (job_t *)arg;

    /* Reservamos resultado en heap para retornarlo por pthread_join. */
    long *result = malloc(sizeof(*result));
    if (!result) {
        return NULL;
    }

    *result = 0;
    for (int x = j->from; x <= j->to; ++x) {
        *result += (long)x * (long)x;
    }

    printf("thread=%d from=%d to=%d partial=%ld\n", j->tid, j->from, j->to, *result);
    return result;
}

int main(void) {
    enum { NUM_THREADS = 3 };

    pthread_t th[NUM_THREADS];
    job_t jobs[NUM_THREADS] = {
        {.tid = 0, .from = 1, .to = 10},
        {.tid = 1, .from = 11, .to = 20},
        {.tid = 2, .from = 21, .to = 30},
    };

    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_create(&th[i], NULL, worker_sum_sq, &jobs[i]) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    long grand_total = 0;
    for (int i = 0; i < NUM_THREADS; ++i) {
        void *ret = NULL;
        if (pthread_join(th[i], &ret) != 0) {
            perror("pthread_join");
            return EXIT_FAILURE;
        }

        /* Si ret es NULL, el hilo no pudo reservar su resultado. */
        if (!ret) {
            fprintf(stderr, "thread %d sin resultado\n", i);
            return EXIT_FAILURE;
        }

        long *partial = (long *)ret;
        /* El hilo principal consolida resultados parciales. */
        grand_total += *partial;
        /* Importante: liberar cada retorno para no filtrar memoria. */
        free(partial);
    }

    /* 1^2 + ... + 30^2 = 9455 */
    printf("grand_total=%ld expected=9455\n", grand_total);
    return (grand_total == 9455) ? EXIT_SUCCESS : EXIT_FAILURE;
}
