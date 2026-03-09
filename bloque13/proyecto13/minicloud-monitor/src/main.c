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

static void make_report(const mon_state_t *st, char *out, size_t out_sz) {
    double avg = (st->total > 0) ? ((double)st->lat_sum / (double)st->total) : 0.0;
    double err_pct = (st->total > 0) ? (100.0 * (double)st->errors / (double)st->total) : 0.0;

    const char *status = "OK";
    if (err_pct >= 30.0) status = "CRIT";
    else if (err_pct >= 10.0 || st->retries > 0) status = "WARN";

    snprintf(out, out_sz,
             "REPORT total=%ld errors=%ld retries=%ld avg_ms=%.2f max_lat=%ld status=%s",
             st->total, st->errors, st->retries, avg, st->max_lat, status);
}

/*
 * EVENT <service> <ok|err> <lat_ms>
 */
static void handle_command(mon_state_t *st, const char *cmd, char *out, size_t out_sz) {
    if (strcmp(cmd, "HEALTH") == 0) {
        snprintf(out, out_sz, "OK");
        return;
    }

    if (strcmp(cmd, "REPORT") == 0) {
        make_report(st, out, out_sz);
        return;
    }

    if (strncmp(cmd, "EVENT ", 6) == 0) {
        char service[64] = {0};
        char result[16] = {0};
        long lat = 0;

        int n = sscanf(cmd + 6, "%63s %15s %ld", service, result, &lat);
        if (n != 3 || lat < 0) {
            snprintf(out, out_sz, "ERR bad_event");
            return;
        }

        st->total++;
        st->lat_sum += lat;
        if (lat > st->max_lat) st->max_lat = lat;

        if (strcmp(result, "ok") != 0) st->errors++;
        if (strcmp(result, "retry") == 0 || strstr(service, "retry") != NULL) st->retries++;

        snprintf(out, out_sz, "ACK");
        return;
    }

    snprintf(out, out_sz, "ERR bad_command");
}

static int run_server(int port) {
    int sfd = start_server(port);
    if (sfd < 0) return -1;

    mon_state_t st;
    memset(&st, 0, sizeof(st));

    for (;;) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) {
            close(sfd);
            return -1;
        }

        char cmd[512];
        if (read_line(cfd, cmd, sizeof(cmd)) >= 0) {
            char resp[512];
            handle_command(&st, cmd, resp, sizeof(resp));
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

    int port = 18084;
    if (argc == 3 && strcmp(argv[1], "--serve") == 0) {
        if (parse_int(argv[2], 1, 65535, &port) != 0) {
            fprintf(stderr, "Puerto invalido\n");
            return EXIT_FAILURE;
        }
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [--serve <port>] | --client <host> <port> <cmd>\n", argv[0]);
        return EXIT_FAILURE;
    }

    return (run_server(port) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
