#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
    long cpu_free;
    long mem_free;
    long max_jobs;
    int have_cpu;
    int have_mem;
    int have_max;
} capacity_t;

typedef struct {
    long accepted;
    long rejected;
} runtime_t;

static int parse_long_range(const char *s, long minv, long maxv, long *out) {
    if (!s || !out || s[0] == '\0') return -1;
    if (s[0] == '+' || s[0] == '-') return -1;

    errno = 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || v < minv || v > maxv) return -1;

    *out = v;
    return 0;
}

static int parse_int(const char *s, int minv, int maxv, int *out) {
    long x = 0;
    if (parse_long_range(s, minv, maxv, &x) != 0) return -1;
    *out = (int)x;
    return 0;
}

static int load_capacity(const char *path, capacity_t *cap) {
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
        if (parse_long_range(v, 1, 1000000, &x) != 0) continue;

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
    return (cap->have_cpu && cap->have_mem && cap->have_max) ? 0 : -1;
}

static int write_all(int fd, const char *buf, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t n = send(fd, buf + off, len - off, 0);
        if (n <= 0) return -1;
        off += (size_t)n;
    }
    return 0;
}

static int read_line(int fd, char *buf, size_t sz) {
    size_t i = 0;
    while (i + 1 < sz) {
        char c;
        ssize_t n = recv(fd, &c, 1, 0);
        if (n == 0) break;
        if (n < 0) return -1;
        if (c == '\n') break;
        if (c != '\r') buf[i++] = c;
    }
    buf[i] = '\0';
    return (int)i;
}

static int start_server(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0) {
        close(fd);
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = htonl(0x7f000001U);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(fd);
        return -1;
    }

    if (listen(fd, 64) != 0) {
        close(fd);
        return -1;
    }

    return fd;
}

static int send_request(const char *host, int port, const char *msg, char *resp, size_t resp_sz) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        close(fd);
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(fd);
        return -1;
    }

    char out[512];
    if (snprintf(out, sizeof(out), "%s\n", msg) >= (int)sizeof(out)) {
        close(fd);
        return -1;
    }

    if (write_all(fd, out, strlen(out)) != 0) {
        close(fd);
        return -1;
    }

    if (read_line(fd, resp, resp_sz) < 0) {
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

/*
 * RUN <job> <cpu_milli> <mem_mb> <timeout_sec> <secret_ref>
 */
static void handle_command(capacity_t *cap, runtime_t *rt, const char *cmd, char *out, size_t out_sz) {
    if (strcmp(cmd, "HEALTH") == 0) {
        snprintf(out, out_sz, "OK");
        return;
    }

    if (strcmp(cmd, "STATS") == 0) {
        snprintf(out, out_sz, "STATS cpu_free=%ld mem_free=%ld accepted=%ld rejected=%ld",
                 cap->cpu_free, cap->mem_free, rt->accepted, rt->rejected);
        return;
    }

    if (strncmp(cmd, "RUN ", 4) == 0) {
        char job[64] = {0};
        char secret[128] = {0};
        long cpu = 0, mem = 0, timeout = 0;

        int n = sscanf(cmd + 4, "%63s %ld %ld %ld %127s", job, &cpu, &mem, &timeout, secret);
        if (n != 5) {
            rt->rejected++;
            snprintf(out, out_sz, "REJECT bad_format");
            return;
        }

        int valid = 1;
        if (cpu < 50 || cpu > 4000) valid = 0;
        if (mem < 64 || mem > 16384) valid = 0;
        if (timeout < 5 || timeout > 3600) valid = 0;
        if (strlen(secret) < 3U) valid = 0;

        if (!valid) {
            rt->rejected++;
            snprintf(out, out_sz, "REJECT invalid_limits");
            return;
        }

        if (rt->accepted >= cap->max_jobs || cpu > cap->cpu_free || mem > cap->mem_free) {
            rt->rejected++;
            snprintf(out, out_sz, "REJECT no_capacity");
            return;
        }

        cap->cpu_free -= cpu;
        cap->mem_free -= mem;
        rt->accepted++;

        snprintf(out, out_sz, "ACCEPT %s", job);
        return;
    }

    snprintf(out, out_sz, "ERR bad_command");
}

static int run_server(int port, capacity_t *cap) {
    int sfd = start_server(port);
    if (sfd < 0) return -1;

    runtime_t rt;
    memset(&rt, 0, sizeof(rt));

    for (;;) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) {
            close(sfd);
            return -1;
        }

        char cmd[512];
        if (read_line(cfd, cmd, sizeof(cmd)) >= 0) {
            char resp[512];
            handle_command(cap, &rt, cmd, resp, sizeof(resp));
            char out[640];
            snprintf(out, sizeof(out), "%s\n", resp);
            (void)write_all(cfd, out, strlen(out));
        }

        close(cfd);
    }
}

int main(int argc, char **argv) {
    if (argc == 5 && strcmp(argv[1], "--client") == 0) {
        const char *host = argv[2];
        int port = 0;
        if (parse_int(argv[3], 1, 65535, &port) != 0) {
            fprintf(stderr, "Puerto invalido\n");
            return EXIT_FAILURE;
        }

        char resp[512];
        if (send_request(host, port, argv[4], resp, sizeof(resp)) != 0) {
            perror("send_request");
            return EXIT_FAILURE;
        }

        printf("%s\n", resp);
        return EXIT_SUCCESS;
    }

    const char *capacity_path = "tests/data/capacity.sample";
    int port = 18083;

    if (argc == 4 && strcmp(argv[1], "--serve") == 0) {
        if (parse_int(argv[2], 1, 65535, &port) != 0) {
            fprintf(stderr, "Puerto invalido\n");
            return EXIT_FAILURE;
        }
        capacity_path = argv[3];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [--serve <port> <capacity>] | --client <host> <port> <cmd>\n", argv[0]);
        return EXIT_FAILURE;
    }

    capacity_t cap;
    if (load_capacity(capacity_path, &cap) != 0) {
        perror("load_capacity");
        return EXIT_FAILURE;
    }

    return (run_server(port, &cap) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
