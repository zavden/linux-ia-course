#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_metrics(const char *path, int *total, int *errors, long *lat_sum) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char svc[64];
    long latency = 0;
    int status = 0;

    *total = 0;
    *errors = 0;
    *lat_sum = 0;

    while (fscanf(f, "%63s %ld %d", svc, &latency, &status) == 3) {
        (void)svc;
        if (latency < 0) continue;

        (*total)++;
        *lat_sum += latency;

        if (status >= 500) {
            (*errors)++;
        }
    }

    fclose(f);
    return 0;
}

static int parse_events(const char *path, int *retries, long *max_lat) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char ts[64], service[64], event[64], result[64];
    long latency = 0;

    *retries = 0;
    *max_lat = 0;

    while (fscanf(f, "%63s %63s %63s %63s %ld", ts, service, event, result, &latency) == 5) {
        (void)ts;
        (void)service;
        (void)result;

        if (strcmp(event, "retry") == 0) {
            (*retries)++;
        }

        if (latency > *max_lat) {
            *max_lat = latency;
        }
    }

    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    const char *metrics_path = "tests/data/metrics.sample";
    const char *events_path = "tests/data/events.sample";

    if (argc == 3) {
        metrics_path = argv[1];
        events_path = argv[2];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [<metrics_file> <events_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int total = 0, errors = 0, retries = 0;
    long lat_sum = 0, max_lat = 0;

    if (parse_metrics(metrics_path, &total, &errors, &lat_sum) != 0 ||
        parse_events(events_path, &retries, &max_lat) != 0) {
        perror("parse_monitor_inputs");
        return EXIT_FAILURE;
    }

    double avg = (total > 0) ? ((double)lat_sum / (double)total) : 0.0;
    double err_pct = (total > 0) ? (100.0 * (double)errors / (double)total) : 0.0;

    const char *status = "OK";
    if (err_pct >= 20.0 || max_lat >= 500) {
        status = "CRIT";
    } else if (err_pct >= 5.0 || retries >= 1) {
        status = "WARN";
    }

    printf("req=%d err=%d err_pct=%.2f avg_ms=%.2f retries=%d max_lat=%ld status=%s\n",
           total, errors, err_pct, avg, retries, max_lat, status);

    return (strcmp(status, "CRIT") == 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
