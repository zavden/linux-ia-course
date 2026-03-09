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

#define MAX_SECRETS 128

typedef struct {
    char name[64];
    char ref[128];
    int active;
} secret_t;

typedef struct {
    secret_t secrets[MAX_SECRETS];
    size_t n;
} secret_store_t;

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

static int load_secrets(const char *path, secret_store_t *st) {
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

static const secret_t *find_secret(const secret_store_t *st, const char *name) {
    for (size_t i = 0; i < st->n; ++i) {
        if (strcmp(st->secrets[i].name, name) == 0) return &st->secrets[i];
    }
    return NULL;
}

static void handle_command(const secret_store_t *st, const char *cmd, char *out, size_t out_sz) {
    if (strcmp(cmd, "HEALTH") == 0) {
        snprintf(out, out_sz, "OK");
        return;
    }

    if (strcmp(cmd, "STATS") == 0) {
        int active = 0;
        int revoked = 0;
        for (size_t i = 0; i < st->n; ++i) {
            if (st->secrets[i].active) active++;
            else revoked++;
        }
        snprintf(out, out_sz, "STATS total=%zu active=%d revoked=%d", st->n, active, revoked);
        return;
    }

    const char *prefix = "GET ";
    if (strncmp(cmd, prefix, strlen(prefix)) == 0) {
        const char *name = cmd + strlen(prefix);
        const secret_t *s = find_secret(st, name);
        if (!s || !s->active) {
            snprintf(out, out_sz, "NOTFOUND");
            return;
        }

        snprintf(out, out_sz, "SECRET %s", s->ref);
        return;
    }

    snprintf(out, out_sz, "ERR bad_command");
}

static int run_server(int port, const secret_store_t *st) {
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
            handle_command(st, cmd, resp, sizeof(resp));
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

    const char *data_path = "tests/data/secrets.sample";
    int port = 18082;

    if (argc == 4 && strcmp(argv[1], "--serve") == 0) {
        if (parse_int(argv[2], 1, 65535, &port) != 0) {
            fprintf(stderr, "Puerto invalido\n");
            return EXIT_FAILURE;
        }
        data_path = argv[3];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [--serve <port> <secrets>] | --client <host> <port> <cmd>\n", argv[0]);
        return EXIT_FAILURE;
    }

    secret_store_t st;
    if (load_secrets(data_path, &st) != 0) {
        perror("load_secrets");
        return EXIT_FAILURE;
    }

    return (run_server(port, &st) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
