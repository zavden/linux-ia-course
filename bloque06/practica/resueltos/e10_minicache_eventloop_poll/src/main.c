#define _POSIX_C_SOURCE 200809L
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define CLIENTS 2
#define MAX_ITEMS 16

typedef struct {
    int used;
    char key[32];
    char value[96];
} kv_t;

typedef struct {
    int cfd;            /* extremo cliente (driver de prueba) */
    int sfd;            /* extremo servidor (event loop) */
    char inbuf[512];    /* buffer de bytes pendientes de parsear */
    size_t inlen;
} conn_t;

static kv_t db[MAX_ITEMS];

static int find_key(const char *key) {
    /* Búsqueda lineal simple: suficiente para escala didáctica. */
    for (int i = 0; i < MAX_ITEMS; ++i) {
        if (db[i].used && strcmp(db[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

static int find_free_slot(void) {
    /* Encuentra primer hueco libre para inserción SET nueva clave. */
    for (int i = 0; i < MAX_ITEMS; ++i) {
        if (!db[i].used) {
            return i;
        }
    }
    return -1;
}

/*
 * Protocolo textual mínimo:
 * - SET <key> <value>  => OK
 * - GET <key>          => VALUE <value> | NULL
 * - DEL <key>          => OK
 */
static void handle_command(const char *line, char *out, size_t out_sz) {
    char cmd[8] = {0};
    char key[32] = {0};
    char val[96] = {0};
    int n = sscanf(line, "%7s %31s %95s", cmd, key, val);

    if (n <= 0) {
        snprintf(out, out_sz, "ERR\n");
        return;
    }

    if (strcmp(cmd, "SET") == 0) {
        if (n != 3) {
            snprintf(out, out_sz, "ERR\n");
            return;
        }

        int idx = find_key(key);
        if (idx == -1) {
            idx = find_free_slot();
            if (idx == -1) {
                snprintf(out, out_sz, "ERR FULL\n");
                return;
            }
            db[idx].used = 1;
            snprintf(db[idx].key, sizeof(db[idx].key), "%s", key);
        }

        snprintf(db[idx].value, sizeof(db[idx].value), "%s", val);
        snprintf(out, out_sz, "OK\n");
        return;
    }

    if (strcmp(cmd, "GET") == 0) {
        if (n != 2) {
            snprintf(out, out_sz, "ERR\n");
            return;
        }

        int idx = find_key(key);
        if (idx == -1) {
            snprintf(out, out_sz, "NULL\n");
        } else {
            snprintf(out, out_sz, "VALUE %s\n", db[idx].value);
        }
        return;
    }

    if (strcmp(cmd, "DEL") == 0) {
        if (n != 2) {
            snprintf(out, out_sz, "ERR\n");
            return;
        }

        int idx = find_key(key);
        if (idx != -1) {
            db[idx].used = 0;
            db[idx].key[0] = '\0';
            db[idx].value[0] = '\0';
        }

        snprintf(out, out_sz, "OK\n");
        return;
    }

    snprintf(out, out_sz, "ERR\n");
}

/*
 * Procesa todas las líneas completas disponibles en una conexión,
 * y responde en el mismo socket servidor.
 */
static int process_connection_buffer(conn_t *c) {
    size_t start = 0;

    for (size_t i = 0; i < c->inlen; ++i) {
        if (c->inbuf[i] != '\n') {
            continue;
        }

        char line[256];
        size_t llen = i - start;
        if (llen >= sizeof(line)) {
            return -1;
        }

        memcpy(line, c->inbuf + start, llen);
        line[llen] = '\0';

        /* Limpieza opcional de CR para entradas estilo CRLF. */
        if (llen > 0 && line[llen - 1] == '\r') {
            line[llen - 1] = '\0';
        }

        char resp[256];
        handle_command(line, resp, sizeof(resp));

        /* Respuesta inmediata (modelo request/response simple). */
        if (write(c->sfd, resp, strlen(resp)) != (ssize_t)strlen(resp)) {
            return -1;
        }

        start = i + 1;
    }

    if (start > 0) {
        /* Conservamos solo remanente incompleto para la próxima lectura. */
        size_t rem = c->inlen - start;
        memmove(c->inbuf, c->inbuf + start, rem);
        c->inlen = rem;
    }

    return 0;
}

/* Lee hasta EOF y convierte '\n' en '|' para salida estable en tests. */
static int read_canonical(int fd, char *out, size_t out_sz) {
    size_t off = 0;

    while (off + 1 < out_sz) {
        ssize_t n = read(fd, out + off, out_sz - off - 1);
        if (n == 0) {
            break;
        }
        if (n < 0) {
            return -1;
        }
        off += (size_t)n;
    }

    out[off] = '\0';

    for (size_t i = 0; out[i] != '\0'; ++i) {
        if (out[i] == '\n') {
            out[i] = '|';
        }
    }

    return 0;
}

int main(void) {
    conn_t conns[CLIENTS];
    memset(conns, 0, sizeof(conns));

    for (int i = 0; i < CLIENTS; ++i) {
        int sv[2];
        /* Por cada cliente: [0]=driver cliente, [1]=servidor reactor. */
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
            perror("socketpair");
            return EXIT_FAILURE;
        }
        conns[i].cfd = sv[0];
        conns[i].sfd = sv[1];
    }

    /*
     * Driver de prueba "cliente": enviamos comandos completos.
     * Cada cliente tiene su propio stream para forzar multiplexación real.
     */
    const char *script0 = "SET nombre Ana\nGET nombre\n";
    const char *script1 = "SET ciudad Lima\nDEL ciudad\nGET ciudad\n";

    if (write(conns[0].cfd, script0, strlen(script0)) != (ssize_t)strlen(script0) ||
        write(conns[1].cfd, script1, strlen(script1)) != (ssize_t)strlen(script1)) {
        perror("write scripts");
        return EXIT_FAILURE;
    }

    /* Señalamos fin de entrada de cliente para que servidor detecte EOF. */
    shutdown(conns[0].cfd, SHUT_WR);
    shutdown(conns[1].cfd, SHUT_WR);

    struct pollfd fds[CLIENTS];
    for (int i = 0; i < CLIENTS; ++i) {
        fds[i].fd = conns[i].sfd;
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }

    int alive = CLIENTS;

    /* Event loop del servidor single-thread. */
    while (alive > 0) {
        int n = poll(fds, CLIENTS, 1000);
        if (n <= 0) {
            perror("poll");
            return EXIT_FAILURE;
        }

        for (int i = 0; i < CLIENTS; ++i) {
            if (fds[i].fd == -1) {
                continue;
            }

            if (fds[i].revents & POLLIN) {
                char tmp[128];
                ssize_t r = read(fds[i].fd, tmp, sizeof(tmp));
                if (r < 0) {
                    perror("read server");
                    return EXIT_FAILURE;
                }

                if (r == 0) {
                    /* EOF: cliente cerró su escritura, cerramos conexión lado servidor. */
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    alive--;
                    continue;
                }

                if (conns[i].inlen + (size_t)r >= sizeof(conns[i].inbuf)) {
                    fprintf(stderr, "input buffer overflow\n");
                    return EXIT_FAILURE;
                }

                memcpy(conns[i].inbuf + conns[i].inlen, tmp, (size_t)r);
                conns[i].inlen += (size_t)r;

                size_t before = conns[i].inlen;
                if (process_connection_buffer(&conns[i]) == -1) {
                    fprintf(stderr, "process_connection_buffer failed\n");
                    return EXIT_FAILURE;
                }
                (void)before;
            }

            if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
                /* Ruta defensiva ante cierre abrupto/error de descriptor. */
                close(fds[i].fd);
                fds[i].fd = -1;
                alive--;
            }
        }
    }

    /* Cerramos escritura del lado servidor para que clientes vean EOF al leer. */
    shutdown(conns[0].sfd, SHUT_WR);
    shutdown(conns[1].sfd, SHUT_WR);

    char resp0[256] = {0};
    char resp1[256] = {0};

    if (read_canonical(conns[0].cfd, resp0, sizeof(resp0)) == -1 ||
        read_canonical(conns[1].cfd, resp1, sizeof(resp1)) == -1) {
        perror("read_canonical");
        return EXIT_FAILURE;
    }

    /* Salida resumida por cliente, normalizada con '|'. */
    printf("resp0=%s resp1=%s\n", resp0, resp1);

    for (int i = 0; i < CLIENTS; ++i) {
        close(conns[i].cfd);
        close(conns[i].sfd);
    }

    int ok = (strcmp(resp0, "OK|VALUE Ana|") == 0 && strcmp(resp1, "OK|OK|NULL|") == 0);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
