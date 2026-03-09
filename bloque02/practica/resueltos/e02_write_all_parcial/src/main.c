#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Tipo de función de escritura para poder inyectar estrategia:
 * - real_write: usa write normal
 * - limited_write: fuerza parciales para testear robustez
 */
typedef ssize_t (*writer_fn)(int fd, const void *buf, size_t n);

/*
 * Escritura artificialmente limitada a 3 bytes por llamada.
 * Esto simula entornos donde write no consume todo el buffer.
 */
static ssize_t limited_write(int fd, const void *buf, size_t n) {
    size_t chunk = n > 3 ? 3 : n;
    return write(fd, buf, chunk);
}

/*
 * write_all_gen:
 * Generaliza el patrón robusto de escritura completa.
 */
static int write_all_gen(int fd, const unsigned char *buf, size_t n, writer_fn fn) {
    size_t off = 0;

    while (off < n) {
        ssize_t w = fn(fd, buf + off, n - off);

        if (w == -1) {
            if (errno == EINTR) continue;
            return -1;
        }

        if (w == 0) {
            errno = EIO;
            return -1;
        }

        off += (size_t)w;
    }

    return 0;
}

int main(int argc, char **argv) {
    int in = -1;
    int out = -1;
    int rc = EXIT_FAILURE;
    unsigned char buf[64];

    if (argc != 3) {
        fprintf(stderr, "Uso: %s <origen> <destino>\n", argv[0]);
        return EXIT_FAILURE;
    }

    in = open(argv[1], O_RDONLY);
    if (in == -1) {
        perror("open in");
        goto cleanup;
    }

    out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out == -1) {
        perror("open out");
        goto cleanup;
    }

    for (;;) {
        ssize_t r = read(in, buf, sizeof(buf));
        if (r == 0) break;
        if (r == -1) {
            if (errno == EINTR) continue;
            perror("read");
            goto cleanup;
        }

        /*
         * Aquí usamos la versión limitada para forzar parciales.
         * Si write_all_gen está bien, el archivo queda íntegro.
         */
        if (write_all_gen(out, buf, (size_t)r, limited_write) == -1) {
            perror("write_all_gen");
            goto cleanup;
        }
    }

    rc = EXIT_SUCCESS;

cleanup:
    if (in != -1) close(in);
    if (out != -1) close(out);
    return rc;
}
