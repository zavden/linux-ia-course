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
    long total;
    long errors;
    long retries;
    long lat_sum;
    long max_lat;
} mon_state_t;

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

static int read_http_request(int fd, char *method, size_t msz, char *path, size_t psz) {
    char line[1024];
    if (read_line(fd, line, sizeof(line)) <= 0) return -1;

    char version[32];
    if (sscanf(line, "%15s %511s %31s", method, path, version) != 3) return -1;

    for (;;) {
        int n = read_line(fd, line, sizeof(line));
        if (n < 0) return -1;
        if (n == 0) break;
    }

    method[msz - 1] = '\0';
    path[psz - 1] = '\0';
    return 0;
}

static int send_http(int fd, int status, const char *body) {
    const char *text = "OK";
    if (status == 200) text = "OK";
    else if (status == 202) text = "Accepted";
    else if (status == 400) text = "Bad Request";
    else if (status == 404) text = "Not Found";

    size_t body_len = strlen(body);
    char hdr[512];
    int n = snprintf(hdr, sizeof(hdr),
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     status, text, body_len);
    if (n < 0 || (size_t)n >= sizeof(hdr)) return -1;

    if (write_all(fd, hdr, (size_t)n) != 0) return -1;
    if (write_all(fd, body, body_len) != 0) return -1;
    return 0;
}

static int query_get(const char *path, const char *key, char *out, size_t out_sz) {
    const char *q = strchr(path, '?');
    if (!q) return -1;
    q++;

    char copy[512];
    snprintf(copy, sizeof(copy), "%s", q);

    char *save = NULL;
    char *pair = strtok_r(copy, "&", &save);
    while (pair) {
        char *eq = strchr(pair, '=');
        if (eq) {
            *eq = '\0';
            if (strcmp(pair, key) == 0) {
                snprintf(out, out_sz, "%s", eq + 1);
                return 0;
            }
        }
        pair = strtok_r(NULL, "&", &save);
    }

    return -1;
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

static void build_report(const mon_state_t *st, char *body, size_t sz) {
    double avg = (st->total > 0) ? ((double)st->lat_sum / (double)st->total) : 0.0;
    double err_pct = (st->total > 0) ? (100.0 * (double)st->errors / (double)st->total) : 0.0;

    const char *status = "OK";
    if (err_pct >= 30.0) status = "CRIT";
    else if (err_pct >= 10.0 || st->retries > 0) status = "WARN";

    snprintf(body, sz,
             "REPORT total=%ld errors=%ld retries=%ld avg_ms=%.2f max_lat=%ld status=%s\n",
             st->total, st->errors, st->retries, avg, st->max_lat, status);
}

static int run_server(int port) {
    int sfd = start_server(port);
    if (sfd < 0) return -1;

    mon_state_t st = {0, 0, 0, 0, 0};

    for (;;) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) {
            close(sfd);
            return -1;
        }

        char method[16] = {0};
        char path[512] = {0};
        if (read_http_request(cfd, method, sizeof(method), path, sizeof(path)) != 0) {
            (void)send_http(cfd, 400, "ERR bad_request\n");
            close(cfd);
            continue;
        }

        if (strcmp(method, "GET") != 0) {
            (void)send_http(cfd, 400, "ERR method_not_allowed\n");
            close(cfd);
            continue;
        }

        if (strcmp(path, "/health") == 0) {
            (void)send_http(cfd, 200, "OK\n");
            close(cfd);
            continue;
        }

        if (strcmp(path, "/report") == 0) {
            char body[256];
            build_report(&st, body, sizeof(body));
            (void)send_http(cfd, 200, body);
            close(cfd);
            continue;
        }

        if (strncmp(path, "/event", 6) == 0) {
            char service[64] = {0};
            char result[32] = {0};
            char lat_s[32] = {0};

            if (query_get(path, "service", service, sizeof(service)) != 0 ||
                query_get(path, "result", result, sizeof(result)) != 0 ||
                query_get(path, "lat_ms", lat_s, sizeof(lat_s)) != 0) {
                (void)send_http(cfd, 400, "ERR missing_params\n");
                close(cfd);
                continue;
            }

            int lat = 0;
            if (parse_int(lat_s, 0, 60000, &lat) != 0) {
                (void)send_http(cfd, 400, "ERR bad_latency\n");
                close(cfd);
                continue;
            }

            st.total++;
            st.lat_sum += lat;
            if (lat > st.max_lat) st.max_lat = lat;
            if (strcmp(result, "ok") != 0) st.errors++;
            if (strcmp(result, "retry") == 0) st.retries++;

            (void)service;
            (void)send_http(cfd, 202, "ACK\n");
            close(cfd);
            continue;
        }

        (void)send_http(cfd, 404, "NOTFOUND\n");
        close(cfd);
    }
}

static int http_client(const char *host, int port, const char *method, const char *path) {
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

    char req[1024];
    int n = snprintf(req, sizeof(req),
                     "%s %s HTTP/1.1\r\n"
                     "Host: %s\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     method, path, host);
    if (n < 0 || (size_t)n >= sizeof(req)) {
        close(fd);
        return -1;
    }

    if (write_all(fd, req, (size_t)n) != 0) {
        close(fd);
        return -1;
    }

    char buf[4096];
    ssize_t r;
    while ((r = recv(fd, buf, sizeof(buf), 0)) > 0) {
        fwrite(buf, 1, (size_t)r, stdout);
    }

    close(fd);
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 6 && strcmp(argv[1], "--client") == 0) {
        int port = 0;
        if (parse_int(argv[3], 1, 65535, &port) != 0) return EXIT_FAILURE;
        return (http_client(argv[2], port, argv[4], argv[5]) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    int port = 28084;
    if (argc == 3 && strcmp(argv[1], "--serve") == 0) {
        if (parse_int(argv[2], 1, 65535, &port) != 0) return EXIT_FAILURE;
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [--serve <port>] | --client <host> <port> <METHOD> <path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    return (run_server(port) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
