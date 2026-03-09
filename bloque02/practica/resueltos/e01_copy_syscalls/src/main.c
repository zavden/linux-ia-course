#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Tamaño de bloque clásico para I/O secuencial básica.
 * 4096 coincide con tamaño de página común en muchos sistemas.
 */
#define BUF_SIZE 4096

/*
 * write_all:
 * Escribe exactamente 'n' bytes salvo error.
 * Este patrón evita bugs cuando write() retorna menos bytes de los pedidos.
 */
static int write_all(int fd, const unsigned char *buf, size_t n) {
    size_t off = 0;

    while (off < n) {
        ssize_t w = write(fd, buf + off, n - off);

        if (w == -1) {
            /* EINTR: llamada interrumpida, reintentamos */
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        /* Protección contra bucle infinito si algo muy extraño devuelve 0 */
        if (w == 0) {
            errno = EIO;
            return -1;
        }

        off += (size_t)w;
    }

    return 0;
}

int main(int argc, char **argv) {
    int fd_in = -1;
    int fd_out = -1;
    int rc = EXIT_FAILURE;
    unsigned char buf[BUF_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Uso: %s <origen> <destino>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Abrimos origen solo lectura */
    fd_in = open(argv[1], O_RDONLY);
    if (fd_in == -1) {
        perror("open origen");
        goto cleanup;
    }

    /*
     * Abrimos/creamos destino:
     * - O_TRUNC: si existe, truncar
     * - 0644: permisos base (luego aplica umask real del proceso)
     */
    fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1) {
        perror("open destino");
        goto cleanup;
    }

    for (;;) {
        /* Leemos hasta BUF_SIZE bytes */
        ssize_t r = read(fd_in, buf, sizeof(buf));

        if (r == 0) {
            /* EOF normal */
            break;
        }

        if (r == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("read");
            goto cleanup;
        }

        /* Escribimos exactamente lo leído */
        if (write_all(fd_out, buf, (size_t)r) == -1) {
            perror("write");
            goto cleanup;
        }
    }

    rc = EXIT_SUCCESS;

cleanup:
    if (fd_in != -1 && close(fd_in) == -1) {
        perror("close origen");
        rc = EXIT_FAILURE;
    }
    if (fd_out != -1 && close(fd_out) == -1) {
        perror("close destino");
        rc = EXIT_FAILURE;
    }

    return rc;
}
