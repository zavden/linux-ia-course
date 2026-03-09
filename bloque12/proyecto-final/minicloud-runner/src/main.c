#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int valid;
    int invalid;
    long cpu_needed;
    long mem_needed;
} job_stats_t;

typedef struct {
    long cpu_free;
    long mem_free;
    long max_jobs;
    int have_cpu;
    int have_mem;
    int have_max;
} capacity_t;

static int valid_name(const char *s) {
    if (!s || s[0] == '\0') return 0;
    size_t len = strlen(s);
    if (len < 3U || len > 40U) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '-' || c == '_' || c == '.')) return 0;
    }
    return 1;
}

static int parse_long_range(const char *s, long minv, long maxv, long *out) {
    if (!s || !out || s[0] == '\0') return 0;
    if (s[0] == '+' || s[0] == '-') return 0;

    errno = 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || v < minv || v > maxv) return 0;

    *out = v;
    return 1;
}

static int parse_jobs(const char *path, job_stats_t *st) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    memset(st, 0, sizeof(*st));

    char line[512];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        char copy[512];
        snprintf(copy, sizeof(copy), "%s", line);

        char *job = strtok(copy, "|");
        char *img = strtok(NULL, "|");
        char *cpu_s = strtok(NULL, "|");
        char *mem_s = strtok(NULL, "|");
        char *to_s = strtok(NULL, "|");
        char *extra = strtok(NULL, "|");

        if (!job || !img || !cpu_s || !mem_s || !to_s || extra) {
            st->invalid++;
            continue;
        }

        long cpu = 0, mem = 0, timeout = 0;
        int ok = valid_name(job) && valid_name(img) &&
                 parse_long_range(cpu_s, 50, 4000, &cpu) &&
                 parse_long_range(mem_s, 64, 16384, &mem) &&
                 parse_long_range(to_s, 5, 3600, &timeout);

        if (!ok) {
            st->invalid++;
            continue;
        }

        st->valid++;
        st->cpu_needed += cpu;
        st->mem_needed += mem;

        (void)timeout;
    }

    fclose(f);
    return 0;
}

static int parse_capacity(const char *path, capacity_t *cap) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    memset(cap, 0, sizeof(*cap));

    char line[256];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char *k = line;
        char *v = eq + 1;

        long x = 0;
        if (!parse_long_range(v, 1, 1000000, &x)) continue;

        if (strcmp(k, "cpu_free_milli") == 0) {
            cap->cpu_free = x;
            cap->have_cpu = 1;
            continue;
        }
        if (strcmp(k, "mem_free_mb") == 0) {
            cap->mem_free = x;
            cap->have_mem = 1;
            continue;
        }
        if (strcmp(k, "max_parallel_jobs") == 0) {
            cap->max_jobs = x;
            cap->have_max = 1;
            continue;
        }
    }

    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    const char *jobs_path = "tests/data/jobs.sample";
    const char *cap_path = "tests/data/capacity.sample";

    if (argc == 3) {
        jobs_path = argv[1];
        cap_path = argv[2];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [<jobs_file> <capacity_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    job_stats_t jobs;
    capacity_t cap;

    if (parse_jobs(jobs_path, &jobs) != 0 || parse_capacity(cap_path, &cap) != 0) {
        perror("parse_runner_inputs");
        return EXIT_FAILURE;
    }

    if (!cap.have_cpu || !cap.have_mem || !cap.have_max) {
        fprintf(stderr, "capacity incompleta\n");
        return EXIT_FAILURE;
    }

    int admitted = 0;
    if (jobs.valid <= cap.max_jobs && jobs.cpu_needed <= cap.cpu_free && jobs.mem_needed <= cap.mem_free) {
        admitted = jobs.valid;
    }

    int ready = (jobs.invalid == 0 && admitted == jobs.valid) ? 1 : 0;

    printf("valid=%d invalid=%d cpu_needed=%ld mem_needed=%ld admitted=%d ready=%d\n",
           jobs.valid, jobs.invalid, jobs.cpu_needed, jobs.mem_needed, admitted, ready);

    return ready ? EXIT_SUCCESS : EXIT_FAILURE;
}
