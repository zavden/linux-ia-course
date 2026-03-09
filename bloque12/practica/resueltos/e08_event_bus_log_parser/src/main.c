#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/events.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<events_log>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    int total = 0, publish = 0, consume = 0, retry = 0, errors = 0;
    long max_lat = 0;

    char ts[64], service[64], event[64], result[64];
    long latency = 0;

    while (fscanf(f, "%63s %63s %63s %63s %ld", ts, service, event, result, &latency) == 5) {
        (void)ts;
        (void)service;

        total++;
        if (strcmp(event, "publish") == 0) publish++;
        else if (strcmp(event, "consume") == 0) consume++;
        else if (strcmp(event, "retry") == 0) retry++;

        if (strcmp(result, "ok") != 0) errors++;

        if (latency > max_lat) {
            max_lat = latency;
        }
    }

    fclose(f);

    printf("total=%d publish=%d consume=%d retry=%d errors=%d max_lat=%ld\n",
           total, publish, consume, retry, errors, max_lat);

    if (argc == 1) {
        return (total == 6 && publish == 2 && consume == 3 && retry == 1 && errors == 2 && max_lat == 240)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
