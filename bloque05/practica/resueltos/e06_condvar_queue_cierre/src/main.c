#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define QUEUE_CAP 8
#define PRODUCERS 2
#define CONSUMERS 3
#define TASKS_PER_PRODUCER 25

typedef struct {
    /* Buffer circular de enteros (tareas). */
    int buf[QUEUE_CAP];
    int head;
    int tail;
    /* Cantidad de elementos actualmente en cola. */
    int count;
} queue_t;

static queue_t q = {0};

static int producers_alive = PRODUCERS;
static int produced_total = 0;
static int consumed_total = 0;
/* checksum para verificar que sí se procesaron valores reales. */
static long long checksum = 0;

static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv_not_empty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cv_not_full = PTHREAD_COND_INITIALIZER;

static void queue_push_unsafe(int v) {
    /* Precondición: hay espacio y mutex tomado por caller. */
    q.buf[q.tail] = v;
    q.tail = (q.tail + 1) % QUEUE_CAP;
    q.count++;
}

static int queue_pop_unsafe(void) {
    /* Precondición: q.count > 0 y mutex tomado por caller. */
    int v = q.buf[q.head];
    q.head = (q.head + 1) % QUEUE_CAP;
    q.count--;
    return v;
}

/*
 * Cada productor emite TASKS_PER_PRODUCER valores únicos.
 * Al terminar decrementa producers_alive y despierta consumidores.
 */
static void *producer(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < TASKS_PER_PRODUCER; ++i) {
        int value = id * 1000 + i;

        pthread_mutex_lock(&m);
        while (q.count == QUEUE_CAP) {
            pthread_cond_wait(&cv_not_full, &m);
        }

        queue_push_unsafe(value);
        produced_total++;

        pthread_cond_signal(&cv_not_empty);
        pthread_mutex_unlock(&m);
    }

    pthread_mutex_lock(&m);
    producers_alive--;
    pthread_cond_broadcast(&cv_not_empty);
    pthread_mutex_unlock(&m);

    return NULL;
}

/*
 * Consumidor:
 * - espera datos si la cola está vacía y aún hay productores vivos
 * - termina cuando cola vacía + producers_alive==0
 */
static void *consumer(void *arg) {
    (void)arg;

    while (1) {
        pthread_mutex_lock(&m);

        while (q.count == 0 && producers_alive > 0) {
            pthread_cond_wait(&cv_not_empty, &m);
        }

        if (q.count == 0 && producers_alive == 0) {
            pthread_mutex_unlock(&m);
            break;
        }

        int value = queue_pop_unsafe();
        consumed_total++;
        checksum += value;

        pthread_cond_signal(&cv_not_full);
        pthread_mutex_unlock(&m);
    }

    return NULL;
}

int main(void) {
    pthread_t prod[PRODUCERS];
    pthread_t cons[CONSUMERS];
    int ids[PRODUCERS];

    /* Arrancamos productores primero; no es obligatorio, pero simplifica trazas. */
    for (int i = 0; i < PRODUCERS; ++i) {
        ids[i] = i + 1;
        if (pthread_create(&prod[i], NULL, producer, &ids[i]) != 0) {
            perror("pthread_create producer");
            return EXIT_FAILURE;
        }
    }

    /* Consumidores drenan en paralelo hasta que ya no queden productores vivos. */
    for (int i = 0; i < CONSUMERS; ++i) {
        if (pthread_create(&cons[i], NULL, consumer, NULL) != 0) {
            perror("pthread_create consumer");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < PRODUCERS; ++i) {
        if (pthread_join(prod[i], NULL) != 0) {
            perror("pthread_join producer");
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < CONSUMERS; ++i) {
        if (pthread_join(cons[i], NULL) != 0) {
            perror("pthread_join consumer");
            return EXIT_FAILURE;
        }
    }

    printf("produced=%d consumed=%d queue_count=%d checksum=%lld\n",
           produced_total,
           consumed_total,
           q.count,
           checksum);

    pthread_cond_destroy(&cv_not_empty);
    pthread_cond_destroy(&cv_not_full);
    pthread_mutex_destroy(&m);

    return (produced_total == PRODUCERS * TASKS_PER_PRODUCER &&
            consumed_total == PRODUCERS * TASKS_PER_PRODUCER &&
            q.count == 0)
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
