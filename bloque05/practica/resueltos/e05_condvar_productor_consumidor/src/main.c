#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TARGET_ITEMS 20
#define CAPACITY 4

/* Estado compartido del \"buffer\" conceptual. */
static int stock = 0;
static int produced = 0;
static int consumed = 0;
/* done=1 indica que no habrá más producción. */
static int done = 0;

static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv_not_empty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cv_not_full = PTHREAD_COND_INITIALIZER;

/*
 * Productor: crea items hasta TARGET_ITEMS.
 * Si el buffer está lleno, se duerme en cv_not_full.
 */
static void *producer(void *arg) {
    (void)arg;

    for (int i = 0; i < TARGET_ITEMS; ++i) {
        pthread_mutex_lock(&m);

        while (stock == CAPACITY) {
            pthread_cond_wait(&cv_not_full, &m);
        }

        stock++;
        produced++;

        /* Avisamos a consumidores que ya hay algo que tomar. */
        pthread_cond_signal(&cv_not_empty);
        pthread_mutex_unlock(&m);

        /* Delay corto para visualizar alternancia productor/consumidor. */
        usleep(1000);
    }

    pthread_mutex_lock(&m);
    done = 1;

    /* Broadcast para despertar consumidores dormidos al finalizar. */
    pthread_cond_broadcast(&cv_not_empty);
    pthread_mutex_unlock(&m);

    return NULL;
}

/*
 * Consumidor: espera mientras no hay stock y el productor no terminó.
 * Cuando done=1 y stock=0, sale limpio.
 */
static void *consumer(void *arg) {
    (void)arg;

    while (1) {
        pthread_mutex_lock(&m);

        while (stock == 0 && !done) {
            pthread_cond_wait(&cv_not_empty, &m);
        }

        if (stock == 0 && done) {
            pthread_mutex_unlock(&m);
            break;
        }

        stock--;
        consumed++;

        /* Avisamos al productor que hay espacio libre. */
        pthread_cond_signal(&cv_not_full);
        pthread_mutex_unlock(&m);
    }

    return NULL;
}

int main(void) {
    pthread_t tp;
    pthread_t tc;

    /* Creamos exactamente un productor y un consumidor. */
    if (pthread_create(&tp, NULL, producer, NULL) != 0) {
        perror("pthread_create producer");
        return EXIT_FAILURE;
    }
    if (pthread_create(&tc, NULL, consumer, NULL) != 0) {
        perror("pthread_create consumer");
        return EXIT_FAILURE;
    }

    if (pthread_join(tp, NULL) != 0 || pthread_join(tc, NULL) != 0) {
        perror("pthread_join");
        return EXIT_FAILURE;
    }

    /* Invariantes esperadas al finalizar correctamente. */
    printf("produced=%d consumed=%d remaining=%d done=%d\n", produced, consumed, stock, done);

    pthread_cond_destroy(&cv_not_empty);
    pthread_cond_destroy(&cv_not_full);
    pthread_mutex_destroy(&m);

    return (produced == TARGET_ITEMS && consumed == TARGET_ITEMS && stock == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
