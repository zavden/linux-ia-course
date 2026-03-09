#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/metrics.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<metrics_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    int total = 0;
    int errors = 0;
    long lat_sum = 0;

    char service[64];
    long latency = 0;
    int status = 0;

    while (fscanf(f, "%63s %ld %d", service, &latency, &status) == 3) {
        (void)service;
        if (latency < 0) {
            continue;
        }

        total++;
        lat_sum += latency;

        if (status >= 500) {
            errors++;
        }
    }

    fclose(f);

    double avg = (total > 0) ? ((double)lat_sum / (double)total) : 0.0;
    double err_pct = (total > 0) ? (100.0 * (double)errors / (double)total) : 0.0;

    printf("total=%d errors=%d avg_ms=%.2f err_pct=%.2f\n", total, errors, avg, err_pct);

    if (argc == 1) {
        return (total == 6 && errors == 2 && avg > 76.66 && avg < 76.67 && err_pct > 33.33 && err_pct < 33.34)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
