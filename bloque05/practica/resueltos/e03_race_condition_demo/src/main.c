#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 2
#define ITER 50000

typedef struct {
    pthread_mutex_t m;
    pthread_cond_t cv;
    /* Número de hilos que deben llegar para liberar la barrera. */
    int trip;
    /* Hilos actualmente esperando en esta ronda. */
    int waiting;
    /* Fase/época de la barrera (permite reutilizarla en loop). */
    int phase;
} barrier_t;

static long long shared_counter = 0;

/*
 * Barrera reusable para sincronizar fases entre hilos.
 * Nos permite forzar que ambos lean antes de escribir,
 * creando pérdida de updates de forma totalmente reproducible.
 */
static int barrier_init(barrier_t *b) {
    if (pthread_mutex_init(&b->m, NULL) != 0) return -1;
    if (pthread_cond_init(&b->cv, NULL) != 0) {
        pthread_mutex_destroy(&b->m);
        return -1;
    }
    b->trip = 0;
    b->waiting = 0;
    b->phase = 0;
    return 0;
}

static void barrier_destroy(barrier_t *b) {
    pthread_cond_destroy(&b->cv);
    pthread_mutex_destroy(&b->m);
}

static void barrier_wait(barrier_t *b) {
    pthread_mutex_lock(&b->m);

    int my_phase = b->phase;
    b->waiting++;

    if (b->waiting == b->trip) {
        b->waiting = 0;
        b->phase++;
        pthread_cond_broadcast(&b->cv);
    } else {
        while (my_phase == b->phase) {
            pthread_cond_wait(&b->cv, &b->m);
        }
    }

    pthread_mutex_unlock(&b->m);
}

typedef struct {
    /* Barrera para sincronizar fin de la etapa READ. */
    barrier_t *b_read;
    /* Barrera para sincronizar fin de la etapa WRITE. */
    barrier_t *b_write;
} args_t;

/*
 * Patrón intencionalmente inseguro:
 * 1) leo shared_counter
 * 2) espero a que el otro hilo también lea
 * 3) escribo tmp+1
 *
 * Resultado: ambos hilos suelen escribir el mismo valor y se pierde
 * exactamente un incremento por iteración global.
 */
static void *worker_racy(void *arg) {
    args_t *a = (args_t *)arg;

    for (int i = 0; i < ITER; ++i) {
        long long tmp = shared_counter;
        barrier_wait(a->b_read);
        shared_counter = tmp + 1;
        barrier_wait(a->b_write);
    }

    return NULL;
}

int main(void) {
    barrier_t b_read;
    barrier_t b_write;

    /* Inicializamos dos barreras separadas para controlar dos fases. */
    if (barrier_init(&b_read) != 0 || barrier_init(&b_write) != 0) {
        perror("barrier_init");
        return EXIT_FAILURE;
    }

    b_read.trip = NUM_THREADS;
    b_write.trip = NUM_THREADS;

    pthread_t th[NUM_THREADS];
    args_t args = {.b_read = &b_read, .b_write = &b_write};

    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_create(&th[i], NULL, worker_racy, &args) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("pthread_join");
            return EXIT_FAILURE;
        }
    }

    long long expected = (long long)NUM_THREADS * ITER;
    int race_detected = (shared_counter != expected);

    /*
     * Con esta coreografía, el valor observado se vuelve reproducible
     * (expected/2), útil para tests automáticos de teoría de races.
     */
    printf("expected=%lld observed=%lld race_detected=%d\n", expected, shared_counter, race_detected);

    barrier_destroy(&b_read);
    barrier_destroy(&b_write);

    return race_detected ? EXIT_SUCCESS : EXIT_FAILURE;
}
