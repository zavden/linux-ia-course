#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>

#if defined(RLIMIT_AS)
#define MEM_LIMIT_KIND RLIMIT_AS
#define MEM_LIMIT_NAME "RLIMIT_AS"
#elif defined(RLIMIT_DATA)
#define MEM_LIMIT_KIND RLIMIT_DATA
#define MEM_LIMIT_NAME "RLIMIT_DATA"
#else
#error "No hay límite de memoria compatible en esta plataforma"
#endif

int main(void) {
    struct rlimit r;
    /* Consultamos límite de memoria soportado por la plataforma actual. */
    if (getrlimit(MEM_LIMIT_KIND, &r) == -1) {
        perror("getrlimit");
        return EXIT_FAILURE;
    }

    printf("kind=%s before_soft=%llu before_hard=%llu\n",
           MEM_LIMIT_NAME,
           (unsigned long long)r.rlim_cur,
           (unsigned long long)r.rlim_max);

    /*
     * Intentamos fijar soft a 64 MiB cuando sea posible.
     * Si no se puede (política/plataforma), continuamos igualmente.
     */
    const rlim_t sixty_four_mb = 64ULL * 1024ULL * 1024ULL;
    if (r.rlim_max == RLIM_INFINITY || r.rlim_max >= sixty_four_mb) {
        /* Nunca subimos hard; solo pedimos bajar soft para el experimento. */
        r.rlim_cur = sixty_four_mb;
        if (setrlimit(MEM_LIMIT_KIND, &r) == -1) {
            /* Algunas plataformas rechazan ajustes en sandbox/container. */
            perror("setrlimit");
        }
    }

    struct rlimit r2;
    if (getrlimit(MEM_LIMIT_KIND, &r2) == -1) {
        perror("getrlimit-2");
        return EXIT_FAILURE;
    }

    printf("after_soft=%llu\n", (unsigned long long)r2.rlim_cur);

    /*
     * Reserva progresiva para observar el límite en acción:
     * cada iteración pide 4 MiB y toca memoria con memset para forzar commit.
     */
    const size_t chunk = 4 * 1024 * 1024;
    const int max_chunks = 64;
    unsigned char *ptrs[max_chunks];
    int used = 0;

    for (int i = 0; i < max_chunks; ++i) {
        ptrs[i] = malloc(chunk);
        if (!ptrs[i]) {
            /* Punto de fallo típico cuando se alcanza el límite efectivo. */
            printf("malloc_failed_at=%d\n", i);
            break;
        }
        /* "Tocar" memoria evita optimizaciones perezosas del allocator. */
        memset(ptrs[i], 0xAA, chunk);
        used++;
    }

    printf("allocated_chunks=%d\n", used);

    /* Liberamos todo lo reservado para cerrar el ciclo limpio. */
    for (int i = 0; i < used; ++i) {
        free(ptrs[i]);
    }

    return EXIT_SUCCESS;
}
