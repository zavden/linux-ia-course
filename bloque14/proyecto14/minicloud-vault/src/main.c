#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_SECRETS 128

typedef struct {
    char name[64];
    char ref[128];
    int active;
} secret_t;

typedef struct {
    secret_t secrets[MAX_SECRETS];
    size_t n;
} store_t;

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
    else if (status == 404) text = "Not Found";
    else if (status == 400) text = "Bad Request";

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

static int load_secrets(const char *path, store_t *st) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    memset(st, 0, sizeof(*st));

    char line[512];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        char copy[512];
        snprintf(copy, sizeof(copy), "%s", line);

        char *name = strtok(copy, "|");
        char *ref = strtok(NULL, "|");
        char *status = strtok(NULL, "|");
        char *extra = strtok(NULL, "|");

        if (!name || !ref || !status || extra) continue;
        if (st->n >= MAX_SECRETS) continue;

        secret_t *s = &st->secrets[st->n++];
        snprintf(s->name, sizeof(s->name), "%s", name);
        snprintf(s->ref, sizeof(s->ref), "%s", ref);
        s->active = (strcmp(status, "active") == 0) ? 1 : 0;
    }

    fclose(f);
    return 0;
}

static const secret_t *find_secret(const store_t *st, const char *name) {
    for (size_t i = 0; i < st->n; ++i) {
        if (strcmp(st->secrets[i].name, name) == 0) return &st->secrets[i];
    }
    return NULL;
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

static int run_server(int port, const store_t *st) {
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

        if (strcmp(path, "/stats") == 0) {
            int active = 0;
            int revoked = 0;
            for (size_t i = 0; i < st->n; ++i) {
                if (st->secrets[i].active) active++;
                else revoked++;
            }

            char body[256];
            snprintf(body, sizeof(body), "STATS total=%zu active=%d revoked=%d\n", st->n, active, revoked);
            (void)send_http(cfd, 200, body);
            close(cfd);
            continue;
        }

        const char *prefix = "/secret/";
        if (strncmp(path, prefix, strlen(prefix)) == 0) {
            const char *name = path + strlen(prefix);
            const secret_t *s = find_secret(st, name);
            if (!s || !s->active) {
                (void)send_http(cfd, 404, "NOTFOUND\n");
                close(cfd);
                continue;
            }

            char body[256];
            snprintf(body, sizeof(body), "SECRET %s\n", s->ref);
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
        if (parse_int(argv[3], 1, 65535, &port) != 0) return EXIT_FAILURE;
        return (http_client(argv[2], port, argv[4], argv[5]) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    const char *data = "tests/data/secrets.sample";
    int port = 28082;

    if (argc == 4 && strcmp(argv[1], "--serve") == 0) {
        if (parse_int(argv[2], 1, 65535, &port) != 0) return EXIT_FAILURE;
        data = argv[3];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [--serve <port> <secrets>] | --client <host> <port> <METHOD> <path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    store_t st;
    if (load_secrets(data, &st) != 0) {
        perror("load_secrets");
        return EXIT_FAILURE;
    }

    return (run_server(port, &st) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
