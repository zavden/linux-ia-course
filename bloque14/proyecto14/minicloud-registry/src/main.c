#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_ROUTES 128

typedef struct {
    char prefix[128];
    char backend[64];
    char host[64];
    int port;
    int up;
} route_t;

typedef struct {
    route_t routes[MAX_ROUTES];
    size_t n_routes;
} route_table_t;

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

    /* Consumir headers hasta linea vacia. */
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
    else if (status == 400) text = "Bad Request";
    else if (status == 404) text = "Not Found";
    else if (status == 500) text = "Internal Server Error";

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

static int load_routes(const char *path, route_table_t *tbl) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    memset(tbl, 0, sizeof(*tbl));

    char line[512];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        char copy[512];
        snprintf(copy, sizeof(copy), "%s", line);

        char *prefix = strtok(copy, "|");
        char *backend = strtok(NULL, "|");
        char *host = strtok(NULL, "|");
        char *port_s = strtok(NULL, "|");
        char *status = strtok(NULL, "|");
        char *extra = strtok(NULL, "|");

        if (!prefix || !backend || !host || !port_s || !status || extra) continue;
        if (tbl->n_routes >= MAX_ROUTES) continue;

        int port = 0;
        if (parse_int(port_s, 1, 65535, &port) != 0) continue;

        route_t *r = &tbl->routes[tbl->n_routes++];
        snprintf(r->prefix, sizeof(r->prefix), "%s", prefix);
        snprintf(r->backend, sizeof(r->backend), "%s", backend);
        snprintf(r->host, sizeof(r->host), "%s", host);
        r->port = port;
        r->up = (strcmp(status, "up") == 0) ? 1 : 0;
    }

    fclose(f);
    return 0;
}

static const route_t *resolve_route(const route_table_t *tbl, const char *path) {
    const route_t *best = NULL;
    size_t best_len = 0U;

    for (size_t i = 0; i < tbl->n_routes; ++i) {
        const route_t *r = &tbl->routes[i];
        size_t len = strlen(r->prefix);
        if (!r->up || len == 0) continue;

        if (strncmp(path, r->prefix, len) == 0 && len > best_len) {
            best = r;
            best_len = len;
        }
    }

    return best;
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

static int run_server(int port, const route_table_t *tbl) {
    int sfd = start_server(port);
    if (sfd < 0) return -1;

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

        if (strcmp(path, "/snapshot") == 0) {
            int up = 0, down = 0;
            for (size_t i = 0; i < tbl->n_routes; ++i) {
                if (tbl->routes[i].up) up++;
                else down++;
            }
            char body[256];
            snprintf(body, sizeof(body), "SNAPSHOT total=%zu up=%d down=%d\n", tbl->n_routes, up, down);
            (void)send_http(cfd, 200, body);
            close(cfd);
            continue;
        }

        if (strncmp(path, "/resolve", 8) == 0) {
            char want[256];
            if (query_get(path, "path", want, sizeof(want)) != 0) {
                (void)send_http(cfd, 400, "ERR missing_path\n");
                close(cfd);
                continue;
            }

            const route_t *r = resolve_route(tbl, want);
            if (!r) {
                (void)send_http(cfd, 404, "NOTFOUND\n");
                close(cfd);
                continue;
            }

            char body[256];
            snprintf(body, sizeof(body), "BACKEND %s %s %d\n", r->backend, r->host, r->port);
            (void)send_http(cfd, 200, body);
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
        if (parse_int(argv[3], 1, 65535, &port) != 0) {
            fprintf(stderr, "Puerto invalido\n");
            return EXIT_FAILURE;
        }
        return (http_client(argv[2], port, argv[4], argv[5]) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    const char *routes = "tests/data/routes.sample";
    int port = 28081;

    if (argc == 4 && strcmp(argv[1], "--serve") == 0) {
        if (parse_int(argv[2], 1, 65535, &port) != 0) return EXIT_FAILURE;
        routes = argv[3];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [--serve <port> <routes>] | --client <host> <port> <METHOD> <path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    route_table_t tbl;
    if (load_routes(routes, &tbl) != 0) {
        perror("load_routes");
        return EXIT_FAILURE;
    }

    return (run_server(port, &tbl) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
