#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

int main(void) {
    struct rlimit r;

    /* Leemos límites actuales de descriptores para este proceso. */
    if (getrlimit(RLIMIT_NOFILE, &r) == -1) {
        perror("getrlimit");
        return EXIT_FAILURE;
    }

    printf("before_soft=%llu before_hard=%llu\n",
           (unsigned long long)r.rlim_cur,
           (unsigned long long)r.rlim_max);

    /*
     * Bajamos soft limit a un valor seguro, sin tocar hard.
     * Solo reducimos si el soft actual es mayor, para evitar EINVAL.
     */
    rlim_t target = (r.rlim_cur > 64) ? 64 : r.rlim_cur;
    r.rlim_cur = target;

    /* setrlimit impacta solo al proceso actual e hijos futuros. */
    if (setrlimit(RLIMIT_NOFILE, &r) == -1) {
        perror("setrlimit");
        return EXIT_FAILURE;
    }

    /* Relectura para comprobar que el kernel aceptó el cambio. */
    struct rlimit r2;
    if (getrlimit(RLIMIT_NOFILE, &r2) == -1) {
        perror("getrlimit-2");
        return EXIT_FAILURE;
    }

    printf("after_soft=%llu after_hard=%llu\n",
           (unsigned long long)r2.rlim_cur,
           (unsigned long long)r2.rlim_max);

    return EXIT_SUCCESS;
}
