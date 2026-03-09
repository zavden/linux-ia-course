#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    int sv[2];

    /*
     * socketpair crea dos FDs conectados entre sí.
     * Ambos son full-duplex y útiles para pruebas locales deterministas.
     */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        perror("socketpair");
        return EXIT_FAILURE;
    }

    const char *ping = "ping";
    /* Buffer de recepción del extremo B (lo que llega desde A). */
    char recv_a[32] = {0};
    /* Buffer de recepción del extremo A (respuesta desde B). */
    char recv_b[32] = {0};

    /* Extremo A -> extremo B */
    ssize_t w1 = write(sv[0], ping, strlen(ping));
    if (w1 != (ssize_t)strlen(ping)) {
        perror("write ping");
        close(sv[0]);
        close(sv[1]);
        return EXIT_FAILURE;
    }

    ssize_t r1 = read(sv[1], recv_a, sizeof(recv_a) - 1);
    if (r1 <= 0) {
        perror("read ping");
        close(sv[0]);
        close(sv[1]);
        return EXIT_FAILURE;
    }

    /* Extremo B -> extremo A */
    const char *pong = "pong";
    /* Segunda mitad del intercambio: ahora B responde hacia A. */
    ssize_t w2 = write(sv[1], pong, strlen(pong));
    if (w2 != (ssize_t)strlen(pong)) {
        perror("write pong");
        close(sv[0]);
        close(sv[1]);
        return EXIT_FAILURE;
    }

    ssize_t r2 = read(sv[0], recv_b, sizeof(recv_b) - 1);
    if (r2 <= 0) {
        perror("read pong");
        close(sv[0]);
        close(sv[1]);
        return EXIT_FAILURE;
    }

    /* Mostramos ambos sentidos para confirmar full-duplex real. */
    printf("got_left=%s got_right=%s\n", recv_a, recv_b);

    /* Cierre explícito de ambos FDs para evitar fugas de descriptores. */
    close(sv[0]);
    close(sv[1]);
    return EXIT_SUCCESS;
}
