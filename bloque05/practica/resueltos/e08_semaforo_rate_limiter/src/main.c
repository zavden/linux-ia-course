#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define THREADS 10
#define CAPACITY 3

typedef enum {
    LIMITER_UNNAMED,
    LIMITER_NAMED,
} limiter_mode_t;

typedef struct {
    limiter_mode_t mode;
    /* Opción rápida: semáforo anónimo en memoria de proceso. */
    sem_t unnamed;
    /* Fallback: semáforo nombrado (útil en plataformas sin sem_init). */
    sem_t *named;
    char sem_name[64];
} limiter_t;

static limiter_t limiter;

static int active_now = 0;
static int max_active = 0;
static int done_threads = 0;

/* Protege métricas compartidas para mantener consistencia de conteos. */
static pthread_mutex_t stats_m = PTHREAD_MUTEX_INITIALIZER;

/*
 * Intentamos primero sem_init (rápido y sin nombre global).
 * Si la plataforma lo bloquea (ENOSYS/EPERM), caemos a sem_open nombrado.
 */
static int limiter_init(limiter_t *l, unsigned int capacity) {
    memset(l, 0, sizeof(*l));

#if !defined(__APPLE__)
    if (sem_init(&l->unnamed, 0, capacity) == 0) {
        l->mode = LIMITER_UNNAMED;
        return 0;
    }

    int saved = errno;
    if (saved != ENOSYS && saved != EPERM && saved != EINVAL) {
        errno = saved;
        return -1;
    }
#endif

    snprintf(l->sem_name, sizeof(l->sem_name), "/b05_e08_%ld", (long)getpid());

    sem_unlink(l->sem_name);
    l->named = sem_open(l->sem_name, O_CREAT | O_EXCL, 0600, capacity);
    if (l->named == SEM_FAILED) {
        return -1;
    }

    l->mode = LIMITER_NAMED;
    return 0;
}

static int limiter_wait(limiter_t *l) {
    sem_t *s = (l->mode == LIMITER_UNNAMED) ? &l->unnamed : l->named;

    while (sem_wait(s) == -1) {
        if (errno == EINTR) {
            /* Si una señal interrumpe el wait, reintentamos. */
            continue;
        }
        return -1;
    }

    return 0;
}

static int limiter_post(limiter_t *l) {
    sem_t *s = (l->mode == LIMITER_UNNAMED) ? &l->unnamed : l->named;
    return sem_post(s);
}

static void limiter_destroy(limiter_t *l) {
    if (l->mode == LIMITER_UNNAMED) {
#if !defined(__APPLE__)
        sem_destroy(&l->unnamed);
#endif
    } else {
        sem_close(l->named);
        sem_unlink(l->sem_name);
    }
}

/*
 * Cada hilo simula una petición:
 * - espera "token" del semáforo
 * - entra a sección crítica acotada
 * - libera token al salir
 */
static void *request_worker(void *arg) {
    (void)arg;

    if (limiter_wait(&limiter) != 0) {
        return NULL;
    }

    pthread_mutex_lock(&stats_m);
    active_now++;
    if (active_now > max_active) {
        max_active = active_now;
    }
    pthread_mutex_unlock(&stats_m);

    usleep(20000);

    pthread_mutex_lock(&stats_m);
    active_now--;
    done_threads++;
    pthread_mutex_unlock(&stats_m);

    limiter_post(&limiter);
    return NULL;
}

int main(void) {
    /* Inicializamos con capacidad 3: máximo tres hilos concurrentes adentro. */
    if (limiter_init(&limiter, CAPACITY) != 0) {
        perror("limiter_init");
        return EXIT_FAILURE;
    }

    pthread_t th[THREADS];

    for (int i = 0; i < THREADS; ++i) {
        if (pthread_create(&th[i], NULL, request_worker, NULL) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < THREADS; ++i) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("pthread_join");
            return EXIT_FAILURE;
        }
    }

    const char *mode = (limiter.mode == LIMITER_UNNAMED) ? "unnamed" : "named";
    printf("threads_done=%d max_active=%d capacity=%d mode=%s\n",
           done_threads,
           max_active,
           CAPACITY,
           mode);

    /* Limpieza explícita de semáforo según modo usado. */
    limiter_destroy(&limiter);
    pthread_mutex_destroy(&stats_m);

    return (done_threads == THREADS && max_active <= CAPACITY) ? EXIT_SUCCESS : EXIT_FAILURE;
}
