#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/statvfs.h>

/* Convierte bloques * tamaño-bloque a kibibytes, evitando overflow simple. */
static uint64_t blocks_to_kib(uint64_t blocks, uint64_t block_size) {
    uint64_t bytes = blocks * block_size;
    return bytes / 1024ULL;
}

int main(int argc, char **argv) {
    /* Ruta objetivo: por defecto el directorio actual. */
    const char *path = (argc >= 2) ? argv[1] : ".";

    struct statvfs st;
    if (statvfs(path, &st) != 0) {
        perror("statvfs");
        return EXIT_FAILURE;
    }

    /* f_frsize es el tamaño real de bloque para cálculos de espacio. */
    uint64_t total_kib = blocks_to_kib((uint64_t)st.f_blocks, (uint64_t)st.f_frsize);
    uint64_t avail_kib = blocks_to_kib((uint64_t)st.f_bavail, (uint64_t)st.f_frsize);
    /* Usado aproximado desde perspectiva de usuario no privilegiado. */
    uint64_t used_kib = (total_kib >= avail_kib) ? (total_kib - avail_kib) : 0;

    /* Porcentaje con doble para evitar truncamiento entero. */
    double used_pct = (total_kib > 0) ? (100.0 * (double)used_kib / (double)total_kib) : 0.0;

    /* Salida estable orientada a scripts de monitoreo. */
    printf("path=%s total_kib=%" PRIu64 " avail_kib=%" PRIu64 " used_pct=%.2f\n",
           path,
           total_kib,
           avail_kib,
           used_pct);

    return (total_kib > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
