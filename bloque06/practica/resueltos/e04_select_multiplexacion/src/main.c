#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

int main(void) {
    int p1[2];
    int p2[2];

    if (pipe(p1) == -1 || pipe(p2) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    /*
     * Solo p2 recibe datos; p1 queda vacío para probar discriminación.
     * select debe marcar listo únicamente el descriptor con bytes pendientes.
     */
    const char *msg = "ready_two";
    if (write(p2[1], msg, strlen(msg)) != (ssize_t)strlen(msg)) {
        perror("write p2");
        close(p1[0]); close(p1[1]);
        close(p2[0]); close(p2[1]);
        return EXIT_FAILURE;
    }

    fd_set rfds;
    /* Construimos conjunto de lectura de esta iteración. */
    FD_ZERO(&rfds);
    FD_SET(p1[0], &rfds);
    FD_SET(p2[0], &rfds);

    /* select necesita maxfd+1, no cantidad de FDs. */
    int maxfd = (p1[0] > p2[0]) ? p1[0] : p2[0];

    struct timeval tv;
    /* Timeout corto para evitar bloqueo indefinido en caso de bug. */
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    int n = select(maxfd + 1, &rfds, NULL, NULL, &tv);
    if (n == -1) {
        perror("select");
        close(p1[0]); close(p1[1]);
        close(p2[0]); close(p2[1]);
        return EXIT_FAILURE;
    }

    int ready1 = FD_ISSET(p1[0], &rfds) ? 1 : 0;
    int ready2 = FD_ISSET(p2[0], &rfds) ? 1 : 0;

    char out[64] = {0};
    if (ready2) {
        /* Leemos solo del FD listo para confirmar ruta correcta. */
        ssize_t r = read(p2[0], out, sizeof(out) - 1);
        if (r <= 0) {
            perror("read p2");
            close(p1[0]); close(p1[1]);
            close(p2[0]); close(p2[1]);
            return EXIT_FAILURE;
        }
    }

    /* Resultado compacto ideal para validación automatizada. */
    printf("select_n=%d ready1=%d ready2=%d msg=%s\n", n, ready1, ready2, out);

    close(p1[0]); close(p1[1]);
    close(p2[0]); close(p2[1]);

    return (ready1 == 0 && ready2 == 1 && strcmp(out, "ready_two") == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
