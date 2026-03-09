#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define WORKERS 4
#define TASKS 20
#define QUEUE_CAP 32

typedef struct {
    /* Cola circular de tareas enteras. */
    int buf[QUEUE_CAP];
    int head;
    int tail;
    int count;
    /* stop=1 indica que no se encolarán más tareas. */
    int stop;

    pthread_mutex_t m;
    pthread_cond_t cv_not_empty;
    pthread_cond_t cv_not_full;
} queue_t;

static queue_t q = {
    .head = 0,
    .tail = 0,
    .count = 0,
    .stop = 0,
    .m = PTHREAD_MUTEX_INITIALIZER,
    .cv_not_empty = PTHREAD_COND_INITIALIZER,
    .cv_not_full = PTHREAD_COND_INITIALIZER,
};

/* Métricas globales de trabajo total procesado por pool. */
static long long sum_sq = 0;
static int processed = 0;

static pthread_mutex_t stats_m = PTHREAD_MUTEX_INITIALIZER;

/*
 * Inserta una tarea en cola bloqueando si está llena.
 */
static void enqueue_task(int value) {
    pthread_mutex_lock(&q.m);

    while (q.count == QUEUE_CAP) {
        pthread_cond_wait(&q.cv_not_full, &q.m);
    }

    q.buf[q.tail] = value;
    q.tail = (q.tail + 1) % QUEUE_CAP;
    q.count++;

    pthread_cond_signal(&q.cv_not_empty);
    pthread_mutex_unlock(&q.m);
}

/*
 * Intenta extraer una tarea:
 * - devuelve 1 si obtuvo valor
 * - devuelve 0 si no hay más trabajo y stop está activo
 */
static int dequeue_task(int *out_value) {
    pthread_mutex_lock(&q.m);

    while (q.count == 0 && !q.stop) {
        pthread_cond_wait(&q.cv_not_empty, &q.m);
    }

    if (q.count == 0 && q.stop) {
        pthread_mutex_unlock(&q.m);
        return 0;
    }

    *out_value = q.buf[q.head];
    q.head = (q.head + 1) % QUEUE_CAP;
    q.count--;

    pthread_cond_signal(&q.cv_not_full);
    pthread_mutex_unlock(&q.m);
    return 1;
}

static void *worker(void *arg) {
    (void)arg;

    while (1) {
        int value = 0;
        /* Si dequeue_task devuelve 0, el pool está en fase de apagado. */
        if (!dequeue_task(&value)) {
            break;
        }

        /* Simulación de procesamiento: cuadrado de la tarea. */
        long long contribution = (long long)value * (long long)value;

        pthread_mutex_lock(&stats_m);
        sum_sq += contribution;
        processed++;
        pthread_mutex_unlock(&stats_m);
    }

    return NULL;
}

int main(void) {
    pthread_t th[WORKERS];

    /* Arrancamos workers \"inmortales\" que esperan tareas en la cola. */
    for (int i = 0; i < WORKERS; ++i) {
        if (pthread_create(&th[i], NULL, worker, NULL) != 0) {
            perror("pthread_create worker");
            return EXIT_FAILURE;
        }
    }

    /* Productor principal: encola tareas 1..TASKS */
    for (int x = 1; x <= TASKS; ++x) {
        enqueue_task(x);
    }

    /* Señal de stop + broadcast para liberar workers dormidos en not_empty. */
    pthread_mutex_lock(&q.m);
    q.stop = 1;
    pthread_cond_broadcast(&q.cv_not_empty);
    pthread_mutex_unlock(&q.m);

    for (int i = 0; i < WORKERS; ++i) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("pthread_join worker");
            return EXIT_FAILURE;
        }
    }

    printf("workers=%d tasks=%d processed=%d sum_sq=%lld\n", WORKERS, TASKS, processed, sum_sq);

    pthread_cond_destroy(&q.cv_not_empty);
    pthread_cond_destroy(&q.cv_not_full);
    pthread_mutex_destroy(&q.m);
    pthread_mutex_destroy(&stats_m);

    return (processed == TASKS && sum_sq == 2870) ? EXIT_SUCCESS : EXIT_FAILURE;
}
