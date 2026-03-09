#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define READERS 4
#define WRITERS 2
#define READ_ITERS 20
#define WRITE_ITERS 5

typedef struct {
    /* Valor compartido que mutan los escritores. */
    int value;
    /* Versión incremental de cambios de escritura. */
    int version;
} cache_t;

static cache_t g_cache = {0, 0};
static long read_ops = 0;
static long write_ops = 0;

/* rwlock para datos de caché; mutex separado para contadores estadísticos. */
static pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;
static pthread_mutex_t stats_m = PTHREAD_MUTEX_INITIALIZER;

/*
 * Lectura concurrente:
 * varios lectores pueden entrar a la vez mientras no haya escritor activo.
 */
static void *reader(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < READ_ITERS; ++i) {
        pthread_rwlock_rdlock(&rw);
        int local_value = g_cache.value;
        int local_version = g_cache.version;
        pthread_rwlock_unlock(&rw);

        pthread_mutex_lock(&stats_m);
        read_ops++;
        pthread_mutex_unlock(&stats_m);

        if (i == 0) {
            printf("reader=%d first_seen_value=%d version=%d\n", id, local_value, local_version);
        }

        usleep(500);
    }

    return NULL;
}

/*
 * Escritura exclusiva:
 * mientras un escritor modifica, nadie más (ni lectores ni escritores) entra.
 */
static void *writer(void *arg) {
    (void)arg;

    for (int i = 0; i < WRITE_ITERS; ++i) {
        pthread_rwlock_wrlock(&rw);
        g_cache.value += 10;
        g_cache.version += 1;
        pthread_rwlock_unlock(&rw);

        pthread_mutex_lock(&stats_m);
        write_ops++;
        pthread_mutex_unlock(&stats_m);

        usleep(1000);
    }

    return NULL;
}

int main(void) {
    pthread_t readers[READERS];
    pthread_t writers[WRITERS];
    int ids[READERS];

    /* Lector/escritor mezclados para observar intercalado real. */
    for (int i = 0; i < READERS; ++i) {
        ids[i] = i;
        if (pthread_create(&readers[i], NULL, reader, &ids[i]) != 0) {
            perror("pthread_create reader");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < WRITERS; ++i) {
        if (pthread_create(&writers[i], NULL, writer, NULL) != 0) {
            perror("pthread_create writer");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < READERS; ++i) {
        if (pthread_join(readers[i], NULL) != 0) {
            perror("pthread_join reader");
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < WRITERS; ++i) {
        if (pthread_join(writers[i], NULL) != 0) {
            perror("pthread_join writer");
            return EXIT_FAILURE;
        }
    }

    printf("final_value=%d version=%d read_ops=%ld write_ops=%ld\n",
           g_cache.value,
           g_cache.version,
           read_ops,
           write_ops);

    pthread_rwlock_destroy(&rw);
    pthread_mutex_destroy(&stats_m);

    return (g_cache.value == 100 && g_cache.version == 10 && write_ops == 10) ? EXIT_SUCCESS : EXIT_FAILURE;
}
