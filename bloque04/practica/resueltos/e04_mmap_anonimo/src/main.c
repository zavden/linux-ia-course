#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

/*
 * Si no existe mapeo anónimo, usamos un archivo temporal en /tmp
 * como respaldo para mantener el ejercicio ejecutable.
 */
static int open_tmp_backing(size_t n) {
    char tpl[] = "/tmp/b04_e04_XXXXXX";
    int fd = mkstemp(tpl);
    if (fd == -1) {
        return -1;
    }

    /* Ya no necesitamos nombre en filesystem: dejamos solo el inode vivo. */
    unlink(tpl);

    if (ftruncate(fd, (off_t)n) == -1) {
        int saved = errno;
        close(fd);
        errno = saved;
        return -1;
    }

    return fd;
}

int main(void) {
    /* 4 KiB para alinear con una página típica del sistema. */
    size_t n = 4096;
    int fd = -1;
    int flags = MAP_PRIVATE;

    /*
     * MAP_ANONYMOUS: no hay archivo asociado.
     * fd = -1 y offset = 0 por contrato POSIX/Linux.
     */
#if defined(MAP_ANONYMOUS)
    flags |= MAP_ANONYMOUS;
#elif defined(MAP_ANON)
    flags |= MAP_ANON;
#else
    /* Fallback portable: mapping privado respaldado por archivo temporal. */
    fd = open_tmp_backing(n);
    if (fd == -1) {
        perror("open_tmp_backing");
        return EXIT_FAILURE;
    }
#endif

    unsigned char *p = mmap(NULL, n, PROT_READ | PROT_WRITE, flags, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap anon");
        if (fd != -1) close(fd);
        return EXIT_FAILURE;
    }

    if (fd != -1) {
        /* El mapping permanece válido aunque cerremos el descriptor. */
        close(fd);
    }

    /* Escribimos patrón repetitivo para comprobar lectura/escritura. */
    for (size_t i = 0; i < n; ++i) {
        p[i] = (unsigned char)(i & 0xFFU);
    }

    /* Mostramos extremos para validar el patrón sin volcar todo el buffer. */
    printf("first=%u last=%u\n", p[0], p[n - 1]);

    /* Liberar mapping devuelve esas páginas al kernel. */
    if (munmap(p, n) == -1) {
        perror("munmap");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
