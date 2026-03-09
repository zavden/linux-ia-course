#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
    char host[64];
    int port;
} endpoint_t;

typedef struct {
    const char *path;
    endpoint_t reg;
    endpoint_t vault;
    endpoint_t runner;
    endpoint_t monitor;
    const char *job;
    int cpu;
    int mem;
    int timeout;
    const char *secret;
} gw_cfg_t;

static int parse_int(const char *s, int minv, int maxv, int *out) {
    if (!s || !out || s[0] == '\0') return -1;
    if (s[0] == '+' || s[0] == '-') return -1;

    errno = 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || v < minv || v > maxv) return -1;

    *out = (int)v;
    return 0;
}

static int parse_endpoint(const char *s, endpoint_t *out) {
    const char *colon = strrchr(s, ':');
    if (!colon) return -1;

    size_t host_len = (size_t)(colon - s);
    if (host_len == 0 || host_len >= sizeof(out->host)) return -1;

    memcpy(out->host, s, host_len);
    out->host[host_len] = '\0';

    return parse_int(colon + 1, 1, 65535, &out->port);
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

static int send_request(const endpoint_t *ep, const char *msg, char *resp, size_t resp_sz) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)ep->port);
    if (inet_pton(AF_INET, ep->host, &addr.sin_addr) != 1) {
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

static void send_monitor_event(const endpoint_t *monitor, const char *result, int lat_ms) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "EVENT gateway %s %d", result, lat_ms);
    char resp[256];
    (void)send_request(monitor, cmd, resp, sizeof(resp));
}

static int run_request(const gw_cfg_t *cfg) {
    char resp[512];
    char cmd[256];

    snprintf(cmd, sizeof(cmd), "RESOLVE %s", cfg->path);
    if (send_request(&cfg->reg, cmd, resp, sizeof(resp)) != 0) {
        fprintf(stderr, "registry_unreachable\n");
        send_monitor_event(&cfg->monitor, "err", 5);
        return EXIT_FAILURE;
    }

    if (strncmp(resp, "BACKEND ", 8) != 0) {
        printf("status=ROUTE_NOT_FOUND path=%s\n", cfg->path);
        send_monitor_event(&cfg->monitor, "err", 5);
        return EXIT_FAILURE;
    }

    char backend[64], host[64];
    int port = 0;
    if (sscanf(resp + 8, "%63s %63s %d", backend, host, &port) != 3) {
        printf("status=ROUTE_ERROR detail=bad_resolve_response\n");
        send_monitor_event(&cfg->monitor, "err", 5);
        return EXIT_FAILURE;
    }

    if (strcmp(backend, "runner") != 0) {
        printf("status=ROUTED backend=%s host=%s port=%d\n", backend, host, port);
        send_monitor_event(&cfg->monitor, "ok", 8);
        return EXIT_SUCCESS;
    }

    snprintf(cmd, sizeof(cmd), "GET %s", cfg->secret);
    if (send_request(&cfg->vault, cmd, resp, sizeof(resp)) != 0 || strncmp(resp, "SECRET ", 7) != 0) {
        printf("status=SECRET_UNAVAILABLE secret=%s\n", cfg->secret);
        send_monitor_event(&cfg->monitor, "err", 12);
        return EXIT_FAILURE;
    }

    const char *ref = resp + 7;

    snprintf(cmd, sizeof(cmd), "RUN %s %d %d %d %s", cfg->job, cfg->cpu, cfg->mem, cfg->timeout, ref);
    if (send_request(&cfg->runner, cmd, resp, sizeof(resp)) != 0) {
        printf("status=RUNNER_UNREACHABLE\n");
        send_monitor_event(&cfg->monitor, "err", 15);
        return EXIT_FAILURE;
    }

    if (strncmp(resp, "ACCEPT ", 7) == 0) {
        printf("status=ACCEPT backend=runner job=%s\n", cfg->job);
        send_monitor_event(&cfg->monitor, "ok", 15);
        return EXIT_SUCCESS;
    }

    printf("status=REJECT backend=runner detail=%s\n", resp);
    send_monitor_event(&cfg->monitor, "err", 20);
    return EXIT_FAILURE;
}

int main(int argc, char **argv) {
    gw_cfg_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    cfg.path = "/run/job-api";
    cfg.job = "job-api";
    cfg.cpu = 500;
    cfg.mem = 512;
    cfg.timeout = 60;
    cfg.secret = "runner_api_key";

    if (parse_endpoint("127.0.0.1:18081", &cfg.reg) != 0 ||
        parse_endpoint("127.0.0.1:18082", &cfg.vault) != 0 ||
        parse_endpoint("127.0.0.1:18083", &cfg.runner) != 0 ||
        parse_endpoint("127.0.0.1:18084", &cfg.monitor) != 0) {
        fprintf(stderr, "default endpoint parse failed\n");
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--request") == 0 && i + 1 < argc) {
            cfg.path = argv[++i];
            continue;
        }
        if (strcmp(argv[i], "--registry") == 0 && i + 1 < argc) {
            if (parse_endpoint(argv[++i], &cfg.reg) != 0) return EXIT_FAILURE;
            continue;
        }
        if (strcmp(argv[i], "--vault") == 0 && i + 1 < argc) {
            if (parse_endpoint(argv[++i], &cfg.vault) != 0) return EXIT_FAILURE;
            continue;
        }
        if (strcmp(argv[i], "--runner") == 0 && i + 1 < argc) {
            if (parse_endpoint(argv[++i], &cfg.runner) != 0) return EXIT_FAILURE;
            continue;
        }
        if (strcmp(argv[i], "--monitor") == 0 && i + 1 < argc) {
            if (parse_endpoint(argv[++i], &cfg.monitor) != 0) return EXIT_FAILURE;
            continue;
        }
        if (strcmp(argv[i], "--job") == 0 && i + 1 < argc) {
            cfg.job = argv[++i];
            continue;
        }
        if (strcmp(argv[i], "--cpu") == 0 && i + 1 < argc) {
            if (parse_int(argv[++i], 50, 4000, &cfg.cpu) != 0) return EXIT_FAILURE;
            continue;
        }
        if (strcmp(argv[i], "--mem") == 0 && i + 1 < argc) {
            if (parse_int(argv[++i], 64, 16384, &cfg.mem) != 0) return EXIT_FAILURE;
            continue;
        }
        if (strcmp(argv[i], "--timeout") == 0 && i + 1 < argc) {
            if (parse_int(argv[++i], 5, 3600, &cfg.timeout) != 0) return EXIT_FAILURE;
            continue;
        }
        if (strcmp(argv[i], "--secret") == 0 && i + 1 < argc) {
            cfg.secret = argv[++i];
            continue;
        }

        fprintf(stderr, "Argumento invalido: %s\n", argv[i]);
        return EXIT_FAILURE;
    }

    return run_request(&cfg);
}
