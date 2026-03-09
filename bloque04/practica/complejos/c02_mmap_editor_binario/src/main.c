#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Reto C02:
 * editor de bytes por offset con mmap.
 */

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <archivo> <offset> <byte_hex>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* TODO 1: parsear offset y byte hex con validación estricta */

    /* TODO 2: open + fstat + validar rango */

    /* TODO 3: mmap MAP_SHARED y escribir byte en offset */

    /* TODO 4: msync + munmap + close */

    puts("TODO: completar editor binario mmap");
    return EXIT_SUCCESS;
}
