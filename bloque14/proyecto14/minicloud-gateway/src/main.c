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
    const char *job;
    const char *secret;
    int cpu;
    int mem;
    int timeout;

    endpoint_t reg;
    endpoint_t vault;
    endpoint_t runner;
    endpoint_t monitor;
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

/*
 * Hace request HTTP simple y devuelve status + body.
 */
static int http_request(const endpoint_t *ep, const char *method, const char *path,
                        int *status, char *body, size_t body_sz) {
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

    char req[1024];
    int n = snprintf(req, sizeof(req),
                     "%s %s HTTP/1.1\r\n"
                     "Host: %s\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     method, path, ep->host);
    if (n < 0 || (size_t)n >= sizeof(req)) {
        close(fd);
        return -1;
    }

    if (write_all(fd, req, (size_t)n) != 0) {
        close(fd);
        return -1;
    }

    char resp[8192];
    size_t used = 0;
    for (;;) {
        ssize_t r = recv(fd, resp + used, sizeof(resp) - used - 1U, 0);
        if (r == 0) break;
        if (r < 0) {
            close(fd);
            return -1;
        }
        used += (size_t)r;
        if (used + 1U >= sizeof(resp)) break;
    }
    close(fd);
    resp[used] = '\0';

    /* Parseo básico de status line. */
    int st = 0;
    if (sscanf(resp, "HTTP/1.1 %d", &st) != 1) {
        return -1;
    }

    const char *sep = strstr(resp, "\r\n\r\n");
    if (!sep) sep = strstr(resp, "\n\n");
    if (!sep) {
        return -1;
    }

    sep += (sep[1] == '\n' && sep[0] == '\n') ? 2 : 4;

    snprintf(body, body_sz, "%s", sep);
    *status = st;
    return 0;
}

static void emit_event(const endpoint_t *monitor, const char *result, int lat_ms) {
    char path[256];
    snprintf(path, sizeof(path), "/event?service=gateway&result=%s&lat_ms=%d", result, lat_ms);

    int st = 0;
    char body[256];
    (void)http_request(monitor, "GET", path, &st, body, sizeof(body));
}

static int run_flow(const gw_cfg_t *cfg) {
    int st = 0;
    char body[512];

    char path[512];
    snprintf(path, sizeof(path), "/resolve?path=%s", cfg->path);

    if (http_request(&cfg->reg, "GET", path, &st, body, sizeof(body)) != 0 || st != 200) {
        printf("status=REGISTRY_ERROR\n");
        emit_event(&cfg->monitor, "err", 5);
        return EXIT_FAILURE;
    }

    char backend[64], host[64];
    int be_port = 0;
    if (sscanf(body, "BACKEND %63s %63s %d", backend, host, &be_port) != 3) {
        printf("status=REGISTRY_MALFORMED\n");
        emit_event(&cfg->monitor, "err", 6);
        return EXIT_FAILURE;
    }

    if (strcmp(backend, "runner") != 0) {
        printf("status=ROUTED backend=%s host=%s port=%d\n", backend, host, be_port);
        emit_event(&cfg->monitor, "ok", 7);
        return EXIT_SUCCESS;
    }

    snprintf(path, sizeof(path), "/secret/%s", cfg->secret);
    if (http_request(&cfg->vault, "GET", path, &st, body, sizeof(body)) != 0 || st != 200) {
        printf("status=SECRET_ERROR name=%s\n", cfg->secret);
        emit_event(&cfg->monitor, "err", 10);
        return EXIT_FAILURE;
    }

    char ref[128];
    if (sscanf(body, "SECRET %127s", ref) != 1) {
        printf("status=SECRET_MALFORMED\n");
        emit_event(&cfg->monitor, "err", 11);
        return EXIT_FAILURE;
    }

    snprintf(path, sizeof(path),
             "/run?job=%s&cpu=%d&mem=%d&timeout=%d&secret=%s",
             cfg->job, cfg->cpu, cfg->mem, cfg->timeout, ref);

    if (http_request(&cfg->runner, "GET", path, &st, body, sizeof(body)) != 0) {
        printf("status=RUNNER_ERROR\n");
        emit_event(&cfg->monitor, "err", 15);
        return EXIT_FAILURE;
    }

    if (st == 200) {
        printf("status=ACCEPT job=%s\n", cfg->job);
        emit_event(&cfg->monitor, "ok", 15);
        return EXIT_SUCCESS;
    }

    printf("status=REJECT detail=%s", body);
    emit_event(&cfg->monitor, "err", 20);
    return EXIT_FAILURE;
}

int main(int argc, char **argv) {
    gw_cfg_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    cfg.path = "/run/job-api";
    cfg.job = "job-api";
    cfg.secret = "runner_api_key";
    cfg.cpu = 500;
    cfg.mem = 512;
    cfg.timeout = 60;

    if (parse_endpoint("127.0.0.1:28081", &cfg.reg) != 0 ||
        parse_endpoint("127.0.0.1:28082", &cfg.vault) != 0 ||
        parse_endpoint("127.0.0.1:28083", &cfg.runner) != 0 ||
        parse_endpoint("127.0.0.1:28084", &cfg.monitor) != 0) {
        fprintf(stderr, "parse default endpoints failed\n");
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
        if (strcmp(argv[i], "--secret") == 0 && i + 1 < argc) {
            cfg.secret = argv[++i];
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

        fprintf(stderr, "Argumento invalido: %s\n", argv[i]);
        return EXIT_FAILURE;
    }

    return run_flow(&cfg);
}
