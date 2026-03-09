#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 8
#define ITER 200000

/* Dato compartido entre todos los hilos del experimento. */
static long long shared_counter = 0;
/* Candado único que serializa acceso al contador. */
static pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * Sección crítica clásica:
 * lock -> modificar estado compartido -> unlock.
 */
static void *worker_safe(void *arg) {
    (void)arg;

    for (int i = 0; i < ITER; ++i) {
        if (pthread_mutex_lock(&counter_lock) != 0) {
            return NULL;
        }
        shared_counter++;
        if (pthread_mutex_unlock(&counter_lock) != 0) {
            return NULL;
        }
    }

    return NULL;
}

int main(void) {
    pthread_t th[NUM_THREADS];

    /* Lanzamos todos los hilos trabajadores. */
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_create(&th[i], NULL, worker_safe, NULL) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    /* Esperamos finalización completa antes de leer shared_counter. */
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("pthread_join");
            return EXIT_FAILURE;
        }
    }

    long long expected = (long long)NUM_THREADS * ITER;
    /* Si mutex protegió bien, observed debe coincidir exacto con expected. */
    int mutex_ok = (shared_counter == expected);

    printf("expected=%lld observed=%lld mutex_ok=%d\n", expected, shared_counter, mutex_ok);

    pthread_mutex_destroy(&counter_lock);
    return mutex_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
