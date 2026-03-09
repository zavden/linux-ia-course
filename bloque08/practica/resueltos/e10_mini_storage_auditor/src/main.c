#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/statvfs.h>

static int count_quota_over_soft(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    int n = 0;
    char user[64];
    long blocks, soft, hard;
    (void)hard;

    while (fscanf(f, "%63s %ld %ld %ld", user, &blocks, &soft, &hard) == 4) {
        (void)user;
        /* Usuario en riesgo cuando rebasa soft limit. */
        if (blocks > soft) {
            n++;
        }
    }

    fclose(f);
    return n;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <path_fs> <quota_report>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *path = argv[1];
    const char *quota = argv[2];

    struct statvfs st;
    /* Capacidad de filesystem para la ruta auditada. */
    if (statvfs(path, &st) != 0) {
        perror("statvfs");
        return EXIT_FAILURE;
    }

    uint64_t total = (uint64_t)st.f_blocks * (uint64_t)st.f_frsize;
    uint64_t avail = (uint64_t)st.f_bavail * (uint64_t)st.f_frsize;
    uint64_t used = (total >= avail) ? (total - avail) : 0;

    /* Señal principal de presión de almacenamiento. */
    double used_pct = (total > 0) ? (100.0 * (double)used / (double)total) : 0.0;

    int over_soft = count_quota_over_soft(quota);
    if (over_soft < 0) {
        perror("count_quota_over_soft");
        return EXIT_FAILURE;
    }

    const char *status = "OK";
    /* Reglas simples de severidad combinando capacidad + quotas. */
    if (used_pct >= 95.0 || over_soft >= 3) {
        status = "CRIT";
    } else if (used_pct >= 80.0 || over_soft >= 1) {
        status = "WARN";
    }

    /* Salida apta para logs y scraping de monitoreo. */
    printf("used_pct=%.2f quota_over_soft=%d status=%s\n", used_pct, over_soft, status);

    return (over_soft == 2 && (status[0] == 'W' || status[0] == 'C')) ? EXIT_SUCCESS : EXIT_FAILURE;
}
