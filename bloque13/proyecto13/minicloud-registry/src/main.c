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

static const route_t *resolve_route(const route_table_t *tbl, const char *path) {
    const route_t *best = NULL;
    size_t best_len = 0;

    for (size_t i = 0; i < tbl->n_routes; ++i) {
        const route_t *r = &tbl->routes[i];
        size_t len = strlen(r->prefix);
        if (len == 0) continue;
        if (!r->up) continue;

        if (strncmp(path, r->prefix, len) == 0 && len > best_len) {
            best = r;
            best_len = len;
        }
    }

    return best;
}

static void handle_command(const route_table_t *tbl, const char *cmd, char *out, size_t out_sz) {
    if (strcmp(cmd, "HEALTH") == 0) {
        snprintf(out, out_sz, "OK");
        return;
    }

    if (strcmp(cmd, "SNAPSHOT") == 0) {
        int up = 0;
        int down = 0;
        for (size_t i = 0; i < tbl->n_routes; ++i) {
            if (tbl->routes[i].up) up++;
            else down++;
        }
        snprintf(out, out_sz, "SNAPSHOT total=%zu up=%d down=%d", tbl->n_routes, up, down);
        return;
    }

    const char *prefix = "RESOLVE ";
    size_t plen = strlen(prefix);
    if (strncmp(cmd, prefix, plen) == 0) {
        const char *path = cmd + plen;
        const route_t *r = resolve_route(tbl, path);
        if (!r) {
            snprintf(out, out_sz, "NOTFOUND");
            return;
        }

        snprintf(out, out_sz, "BACKEND %s %s %d", r->backend, r->host, r->port);
        return;
    }

    snprintf(out, out_sz, "ERR bad_command");
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

        char cmd[512];
        if (read_line(cfd, cmd, sizeof(cmd)) >= 0) {
            char resp[512];
            handle_command(tbl, cmd, resp, sizeof(resp));
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

    const char *routes_path = "tests/data/routes.sample";
    int port = 18081;

    if (argc == 4 && strcmp(argv[1], "--serve") == 0) {
        if (parse_int(argv[2], 1, 65535, &port) != 0) {
            fprintf(stderr, "Puerto invalido\n");
            return EXIT_FAILURE;
        }
        routes_path = argv[3];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [--serve <port> <routes>] | --client <host> <port> <cmd>\n", argv[0]);
        return EXIT_FAILURE;
    }

    route_table_t tbl;
    if (load_routes(routes_path, &tbl) != 0) {
        perror("load_routes");
        return EXIT_FAILURE;
    }

    return (run_server(port, &tbl) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
