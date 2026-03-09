#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int limited;
    long quota;
    long period;
    int weight;
    double ratio_pct;
} cpu_limits_t;

static int parse_cpu_max(const char *path, cpu_limits_t *out) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char quota_s[64] = {0};
    char period_s[64] = {0};
    if (fscanf(f, "%63s %63s", quota_s, period_s) != 2) {
        fclose(f);
        return -1;
    }
    fclose(f);

    errno = 0;
    char *end = NULL;
    long period = strtol(period_s, &end, 10);
    if (errno != 0 || end == period_s || *end != '\0' || period <= 0) {
        return -1;
    }

    out->period = period;

    if (strcmp(quota_s, "max") == 0) {
        out->limited = 0;
        out->quota = -1;
        out->ratio_pct = -1.0;
        return 0;
    }

    errno = 0;
    end = NULL;
    long quota = strtol(quota_s, &end, 10);
    if (errno != 0 || end == quota_s || *end != '\0' || quota <= 0) {
        return -1;
    }

    out->limited = 1;
    out->quota = quota;
    out->ratio_pct = (100.0 * (double)quota) / (double)period;
    return 0;
}

static int parse_cpu_weight(const char *path, cpu_limits_t *out) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    int w = 0;
    if (fscanf(f, "%d", &w) != 1) {
        fclose(f);
        return -1;
    }
    fclose(f);

    if (w < 1 || w > 10000) {
        return -1;
    }

    out->weight = w;
    return 0;
}

int main(int argc, char **argv) {
    const char *cpu_max = "tests/data/cpu.max.sample";
    const char *cpu_weight = "tests/data/cpu.weight.sample";

    if (argc == 3) {
        cpu_max = argv[1];
        cpu_weight = argv[2];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [<cpu.max> <cpu.weight>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    cpu_limits_t lim = {0, 0, 0, 0, 0.0};
    if (parse_cpu_max(cpu_max, &lim) != 0 || parse_cpu_weight(cpu_weight, &lim) != 0) {
        perror("parse_cpu_limits");
        return EXIT_FAILURE;
    }

    printf("limited=%d quota=%ld period=%ld ratio=%.2f weight=%d\n",
           lim.limited, lim.quota, lim.period, lim.ratio_pct, lim.weight);

    if (argc == 1) {
        return (lim.limited == 1 && lim.quota == 50000 && lim.period == 100000 &&
                lim.weight == 100 && lim.ratio_pct > 49.99 && lim.ratio_pct < 50.01)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
