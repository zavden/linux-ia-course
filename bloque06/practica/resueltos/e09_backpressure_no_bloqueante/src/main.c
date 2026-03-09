#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int set_nonblocking(int fd) {
    /* Leemos flags existentes para no sobrescribir otros bits. */
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }
    /* Activamos O_NONBLOCK preservando el resto de configuración. */
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}

int main(void) {
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        perror("socketpair");
        return EXIT_FAILURE;
    }

    if (set_nonblocking(sv[0]) == -1 || set_nonblocking(sv[1]) == -1) {
        perror("set_nonblocking");
        close(sv[0]);
        close(sv[1]);
        return EXIT_FAILURE;
    }

    char chunk[4096];
    /* Patrón fijo; su contenido no importa para el experimento. */
    memset(chunk, 'A', sizeof(chunk));

    size_t wrote_before = 0;
    int first_block = 0;

    /*
     * Intentamos llenar el socket de envío hasta EAGAIN.
     * Esto modela backpressure: el receptor no drena lo bastante rápido.
     */
    for (int i = 0; i < 4096; ++i) {
        ssize_t n = write(sv[0], chunk, sizeof(chunk));
        if (n > 0) {
            wrote_before += (size_t)n;
            continue;
        }
        if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            first_block = 1;
            break;
        }
        perror("write before block");
        close(sv[0]);
        close(sv[1]);
        return EXIT_FAILURE;
    }

    /* Drenamos parte del buffer del lado receptor para liberar espacio. */
    size_t drained = 0;
    while (drained < 65536) {
        char tmp[4096];
        ssize_t r = read(sv[1], tmp, sizeof(tmp));
        if (r > 0) {
            drained += (size_t)r;
            continue;
        }
        if (r == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        }
        if (r == 0) {
            break;
        }
        perror("read drain");
        close(sv[0]);
        close(sv[1]);
        return EXIT_FAILURE;
    }

    size_t wrote_after = 0;
    for (int i = 0; i < 64; ++i) {
        ssize_t n = write(sv[0], chunk, sizeof(chunk));
        if (n > 0) {
            wrote_after += (size_t)n;
            continue;
        }
        if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            /* Volvió a llenarse: se corta esta ronda de reintentos. */
            break;
        }
        perror("write after drain");
        close(sv[0]);
        close(sv[1]);
        return EXIT_FAILURE;
    }

    /* Métricas básicas para observar ciclo saturar->drenar->reanudar. */
    printf("blocked=%d wrote_before=%zu drained=%zu wrote_after=%zu\n",
           first_block,
           wrote_before,
           drained,
           wrote_after);

    close(sv[0]);
    close(sv[1]);

    return (first_block == 1 && wrote_before > 0 && drained > 0 && wrote_after > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
