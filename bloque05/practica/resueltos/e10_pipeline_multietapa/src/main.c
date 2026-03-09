#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define QUEUE_CAP 16
#define ITEMS 15
#define SENTINEL (-1)

typedef struct {
    /* Cola circular thread-safe. */
    int buf[QUEUE_CAP];
    int head;
    int tail;
    int count;
    pthread_mutex_t m;
    pthread_cond_t cv_not_empty;
    pthread_cond_t cv_not_full;
} queue_t;

static queue_t q_in = {
    .m = PTHREAD_MUTEX_INITIALIZER,
    .cv_not_empty = PTHREAD_COND_INITIALIZER,
    .cv_not_full = PTHREAD_COND_INITIALIZER,
};
static queue_t q_mid = {
    .m = PTHREAD_MUTEX_INITIALIZER,
    .cv_not_empty = PTHREAD_COND_INITIALIZER,
    .cv_not_full = PTHREAD_COND_INITIALIZER,
};

/* Estado agregado de salida final del pipeline. */
static long long final_sum = 0;
static int final_count = 0;
static pthread_mutex_t stats_m = PTHREAD_MUTEX_INITIALIZER;

static void queue_push(queue_t *q, int value) {
    pthread_mutex_lock(&q->m);
    /* Backpressure: si cola llena, productor espera espacio. */
    while (q->count == QUEUE_CAP) {
        pthread_cond_wait(&q->cv_not_full, &q->m);
    }

    q->buf[q->tail] = value;
    q->tail = (q->tail + 1) % QUEUE_CAP;
    q->count++;

    pthread_cond_signal(&q->cv_not_empty);
    pthread_mutex_unlock(&q->m);
}

static int queue_pop(queue_t *q) {
    pthread_mutex_lock(&q->m);
    /* Si cola vacía, consumidor duerme sin busy-wait. */
    while (q->count == 0) {
        pthread_cond_wait(&q->cv_not_empty, &q->m);
    }

    int v = q->buf[q->head];
    q->head = (q->head + 1) % QUEUE_CAP;
    q->count--;

    pthread_cond_signal(&q->cv_not_full);
    pthread_mutex_unlock(&q->m);
    return v;
}

/*
 * Etapa 1: consume de q_in y produce en q_mid.
 * Si recibe SENTINEL, lo reenvía para detener la etapa siguiente.
 */
static void *stage1(void *arg) {
    (void)arg;

    while (1) {
        int x = queue_pop(&q_in);
        if (x == SENTINEL) {
            queue_push(&q_mid, SENTINEL);
            break;
        }

        int y = 2 * x;
        queue_push(&q_mid, y);
    }

    return NULL;
}

/*
 * Etapa 2: consume de q_mid, aplica transformación final y acumula métricas.
 */
static void *stage2(void *arg) {
    (void)arg;

    while (1) {
        int y = queue_pop(&q_mid);
        if (y == SENTINEL) {
            break;
        }

        int z = y + 1;

        pthread_mutex_lock(&stats_m);
        final_sum += z;
        final_count++;
        pthread_mutex_unlock(&stats_m);
    }

    return NULL;
}

int main(void) {
    pthread_t t1;
    pthread_t t2;

    /* Levantamos una hebra por etapa del pipeline. */
    if (pthread_create(&t1, NULL, stage1, NULL) != 0) {
        perror("pthread_create stage1");
        return EXIT_FAILURE;
    }
    if (pthread_create(&t2, NULL, stage2, NULL) != 0) {
        perror("pthread_create stage2");
        return EXIT_FAILURE;
    }

    /* Productor del pipeline: ingresa datos iniciales 1..ITEMS. */
    for (int i = 1; i <= ITEMS; ++i) {
        queue_push(&q_in, i);
    }
    /* Sentinel marca fin de stream para disparar apagado ordenado. */
    queue_push(&q_in, SENTINEL);

    if (pthread_join(t1, NULL) != 0 || pthread_join(t2, NULL) != 0) {
        perror("pthread_join");
        return EXIT_FAILURE;
    }

    /* Sum_{i=1..15}(2i+1)=255 */
    printf("items=%d final_count=%d final_sum=%lld expected=255\n", ITEMS, final_count, final_sum);

    pthread_cond_destroy(&q_in.cv_not_empty);
    pthread_cond_destroy(&q_in.cv_not_full);
    pthread_mutex_destroy(&q_in.m);

    pthread_cond_destroy(&q_mid.cv_not_empty);
    pthread_cond_destroy(&q_mid.cv_not_full);
    pthread_mutex_destroy(&q_mid.m);

    pthread_mutex_destroy(&stats_m);

    return (final_count == ITEMS && final_sum == 255) ? EXIT_SUCCESS : EXIT_FAILURE;
}
