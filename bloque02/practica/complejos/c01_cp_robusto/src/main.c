#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Reto C01:
 * Implementar cp robusto real.
 * Este archivo te guía con TODOs, pero no te regala la solución final.
 */

#define BUF_SIZE 65536

/* TODO 1:
 * Implementa write_all que:
 * - reintente en EINTR
 * - maneje parciales
 * - falle con -1 en error real
 */
static int write_all(int fd, const unsigned char *buf, size_t n) {
    (void)fd;
    (void)buf;
    (void)n;
    errno = ENOSYS;
    return -1;
}

int main(int argc, char **argv) {
    int in = -1;
    int out = -1;
    int rc = EXIT_FAILURE;
    unsigned char buf[BUF_SIZE];
    (void)buf;

    /* TODO 2: validar args */
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <origen> <destino>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* TODO 3: abrir origen y destino con flags correctos */
    in = open(argv[1], O_RDONLY);
    if (in == -1) {
        perror("open origen");
        goto cleanup;
    }

    out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out == -1) {
        perror("open destino");
        goto cleanup;
    }

    /* Mantiene la plantilla compilable con -Werror antes de implementar TODO 4 */
    if (0) {
        (void)write_all(out, buf, 0);
    }

    /*
     * TODO 4:
     * bucle read + write_all
     * - tratar EINTR
     * - cerrar limpio en cualquier error
     */

    rc = EXIT_SUCCESS;

cleanup:
    if (in != -1) close(in);
    if (out != -1) close(out);
    return rc;
}
