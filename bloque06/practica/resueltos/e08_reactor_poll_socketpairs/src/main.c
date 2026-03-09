#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define CLIENTS 3

/* Convierte en mayúsculas in-place. */
static void str_upper(char *s) {
    for (size_t i = 0; s[i] != '\0'; ++i) {
        s[i] = (char)toupper((unsigned char)s[i]);
    }
}

/* Lee hasta EOF o tamaño máximo; usado para recoger respuesta cliente. */
static int read_small(int fd, char *out, size_t out_sz) {
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
    return 0;
}

int main(void) {
    int sv[CLIENTS][2];
    for (int i = 0; i < CLIENTS; ++i) {
        /* Cada cliente simulado tiene su propio canal full-duplex. */
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv[i]) == -1) {
            perror("socketpair");
            return EXIT_FAILURE;
        }
    }

    const char *inputs[CLIENTS] = {"hola", "mundo", "reactor"};

    /* Cada cliente envía su payload y cierra escritura. */
    for (int i = 0; i < CLIENTS; ++i) {
        /* cfd escribe petición inicial al reactor servidor. */
        if (write(sv[i][0], inputs[i], strlen(inputs[i])) != (ssize_t)strlen(inputs[i])) {
            perror("write client");
            return EXIT_FAILURE;
        }
        shutdown(sv[i][0], SHUT_WR);
    }

    struct pollfd fds[CLIENTS];
    for (int i = 0; i < CLIENTS; ++i) {
        /* El reactor observa únicamente extremos servidor. */
        fds[i].fd = sv[i][1];
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }

    int handled = 0;

    /* Reactor loop: procesa clientes listos hasta completar todos. */
    while (handled < CLIENTS) {
        int n = poll(fds, CLIENTS, 1000);
        if (n <= 0) {
            perror("poll timeout/error");
            return EXIT_FAILURE;
        }

        for (int i = 0; i < CLIENTS; ++i) {
            if (fds[i].fd == -1) {
                continue;
            }
            if (!(fds[i].revents & POLLIN)) {
                continue;
            }

            char tmp[128] = {0};
            /* Lectura de payload entrante de cliente i. */
            ssize_t r = read(fds[i].fd, tmp, sizeof(tmp) - 1);
            if (r <= 0) {
                continue;
            }

            str_upper(tmp);
            /* Respuesta procesada en el mismo canal (echo transformado). */
            if (write(fds[i].fd, tmp, strlen(tmp)) != (ssize_t)strlen(tmp)) {
                perror("write server");
                return EXIT_FAILURE;
            }

            shutdown(fds[i].fd, SHUT_WR);
            handled++;
            fds[i].fd = -1;
        }
    }

    char out0[64] = {0};
    char out1[64] = {0};
    char out2[64] = {0};

    if (read_small(sv[0][0], out0, sizeof(out0)) == -1 ||
        read_small(sv[1][0], out1, sizeof(out1)) == -1 ||
        read_small(sv[2][0], out2, sizeof(out2)) == -1) {
        perror("read client replies");
        return EXIT_FAILURE;
    }

    /* Verificación final de respuestas por cliente. */
    printf("handled=%d c0=%s c1=%s c2=%s\n", handled, out0, out1, out2);

    for (int i = 0; i < CLIENTS; ++i) {
        close(sv[i][0]);
        close(sv[i][1]);
    }

    return (handled == 3 && strcmp(out0, "HOLA") == 0 && strcmp(out1, "MUNDO") == 0 && strcmp(out2, "REACTOR") == 0)
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
