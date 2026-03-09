#define _POSIX_C_SOURCE 200809L
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    int p1[2];
    int p2[2];

    if (pipe(p1) == -1 || pipe(p2) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    const char *msg = "poll_ok";
    /* Solo p1 recibe datos; p2 queda vacío. */
    if (write(p1[1], msg, strlen(msg)) != (ssize_t)strlen(msg)) {
        perror("write p1");
        close(p1[0]); close(p1[1]);
        close(p2[0]); close(p2[1]);
        return EXIT_FAILURE;
    }

    struct pollfd fds[2];
    memset(fds, 0, sizeof(fds));
    /* Entrada A: debería quedar lista con POLLIN. */
    fds[0].fd = p1[0];
    fds[0].events = POLLIN;
    /* Entrada B: no debería reportar datos. */
    fds[1].fd = p2[0];
    fds[1].events = POLLIN;

    /* poll espera hasta 1s por actividad en cualquiera de los FDs. */
    int n = poll(fds, 2, 1000);
    if (n == -1) {
        perror("poll");
        close(p1[0]); close(p1[1]);
        close(p2[0]); close(p2[1]);
        return EXIT_FAILURE;
    }

    int ready_a = (fds[0].revents & POLLIN) ? 1 : 0;
    int ready_b = (fds[1].revents & POLLIN) ? 1 : 0;

    char out[64] = {0};
    if (ready_a) {
        /* Leemos payload del descriptor que poll marcó listo. */
        ssize_t r = read(p1[0], out, sizeof(out) - 1);
        if (r <= 0) {
            perror("read p1");
            close(p1[0]); close(p1[1]);
            close(p2[0]); close(p2[1]);
            return EXIT_FAILURE;
        }
    }

    /* Trazamos reactividad y contenido para comprobar multiplexación. */
    printf("poll_n=%d ready_a=%d ready_b=%d msg=%s\n", n, ready_a, ready_b, out);

    close(p1[0]); close(p1[1]);
    close(p2[0]); close(p2[1]);

    return (ready_a == 1 && ready_b == 0 && strcmp(out, "poll_ok") == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
