#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * parse_meminfo:
 * lee un stream con formato estilo /proc/meminfo y extrae 4 métricas mínimas.
 * No depende de orden fijo de líneas: escanea hasta EOF.
 */
static int parse_meminfo(FILE *f, long *total_kb, long *free_kb, long *buffers_kb, long *cached_kb) {
    char line[256];
    /* -1 significa "aún no encontrado". */
    *total_kb = *free_kb = *buffers_kb = *cached_kb = -1;

    while (fgets(line, sizeof(line), f) != NULL) {
        /* Cada sscanf intenta un patrón y continúa al siguiente renglón. */
        if (sscanf(line, "MemTotal: %ld kB", total_kb) == 1) continue;
        if (sscanf(line, "MemFree: %ld kB", free_kb) == 1) continue;
        if (sscanf(line, "Buffers: %ld kB", buffers_kb) == 1) continue;
        if (sscanf(line, "Cached: %ld kB", cached_kb) == 1) continue;
    }

    /* Exigimos las 4 claves para considerar parseo completo. */
    return (*total_kb >= 0 && *free_kb >= 0 && *buffers_kb >= 0 && *cached_kb >= 0) ? 0 : -1;
}

int main(int argc, char **argv) {
    const char *path = NULL;

    /*
     * Modo 1: archivo explícito pasado por argumento (ideal para tests).
     * Modo 2: /proc/meminfo real del host si existe y es legible.
     */
    if (argc >= 2) {
        path = argv[1];
    } else if (access("/proc/meminfo", R_OK) == 0) {
        path = "/proc/meminfo";
    } else {
        fprintf(stderr, "Uso: %s <archivo_meminfo>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Abrimos el origen de datos como texto plano. */
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    long total, free_kb, buffers, cached;
    if (parse_meminfo(f, &total, &free_kb, &buffers, &cached) == -1) {
        fclose(f);
        fprintf(stderr, "Formato meminfo incompleto\n");
        return EXIT_FAILURE;
    }

    /* Cerramos el stream cuanto antes; ya tenemos los datos en variables. */
    fclose(f);

    /*
     * Aproximación pedagógica de memoria usada:
     * used = total - free - buffers - cached
     * (en Linux real hay más matices, pero este modelo es útil para empezar).
     */
    long used = total - free_kb - buffers - cached;
    double used_pct = (total > 0) ? (100.0 * (double)used / (double)total) : 0.0;

    /* Salida estable para scripts/tests automatizados. */
    printf("total_kb=%ld used_kb=%ld used_pct=%.2f\n", total, used, used_pct);
    return EXIT_SUCCESS;
}
