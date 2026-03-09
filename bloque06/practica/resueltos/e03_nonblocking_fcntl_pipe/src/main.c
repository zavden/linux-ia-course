#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    int p[2];
    /* p[0]=read end, p[1]=write end. */
    if (pipe(p) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    /* Guardamos flags actuales para restaurarlos al final. */
    int old_flags = fcntl(p[0], F_GETFL, 0);
    if (old_flags == -1) {
        perror("fcntl F_GETFL");
        close(p[0]);
        close(p[1]);
        return EXIT_FAILURE;
    }

    if (fcntl(p[0], F_SETFL, old_flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL O_NONBLOCK");
        close(p[0]);
        close(p[1]);
        return EXIT_FAILURE;
    }

    /* Primera lectura sin datos: debe devolver -1 con EAGAIN/EWOULDBLOCK. */
    char buf[32] = {0};
    errno = 0;
    ssize_t n1 = read(p[0], buf, sizeof(buf));
    /* read debe fallar \"bien\" con EAGAIN al no haber bytes disponibles. */
    int got_eagain = (n1 == -1 && (errno == EAGAIN || errno == EWOULDBLOCK));

    const char *msg = "abc";
    /* Escribimos datos y repetimos read para comprobar progreso real. */
    if (write(p[1], msg, 3) != 3) {
        perror("write pipe");
        close(p[0]);
        close(p[1]);
        return EXIT_FAILURE;
    }

    memset(buf, 0, sizeof(buf));
    ssize_t n2 = read(p[0], buf, sizeof(buf) - 1);
    if (n2 <= 0) {
        perror("read second");
        close(p[0]);
        close(p[1]);
        return EXIT_FAILURE;
    }

    /* Cleanup: restauramos flags previos para no dejar FD "sucio". */
    (void)fcntl(p[0], F_SETFL, old_flags);

    printf("first_try=%s second_read=%s bytes=%zd\n", got_eagain ? "eagain" : "other", buf, n2);

    close(p[0]);
    close(p[1]);

    return (got_eagain && strcmp(buf, "abc") == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
