#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int count_valid_lines(const char *path) {
    /* Reutilizable para mounts/lvs cuando el formato es lineal simple. */
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    int n = 0;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        /* Filtro genérico para comentarios y líneas vacías. */
        size_t i = 0;
        while (line[i] && isspace((unsigned char)line[i])) i++;
        if (line[i] == '\0' || line[i] == '#') continue;
        n++;
    }
    fclose(f);
    return n;
}

static int count_over_soft(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    int n = 0;
    char user[64];
    long blocks, soft, hard;
    (void)hard;

    while (fscanf(f, "%63s %ld %ld %ld", user, &blocks, &soft, &hard) == 4) {
        (void)user;
        /* Política simple: alerta cuando supera soft limit. */
        if (blocks > soft) n++;
    }

    fclose(f);
    return n;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <mounts_file> <lvs_file> <quota_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Recolección de métricas por fuente de inventario. */
    int mounts = count_valid_lines(argv[1]);
    int lvs = count_valid_lines(argv[2]);
    int over_soft = count_over_soft(argv[3]);

    if (mounts < 0 || lvs < 0 || over_soft < 0) {
        perror("inventory parse");
        return EXIT_FAILURE;
    }

    /* Formato compacto tipo JSON para integraciones posteriores. */
    printf("{\"mounts\":%d,\"lvs\":%d,\"quota_over_soft\":%d}\n", mounts, lvs, over_soft);

    return (mounts == 2 && lvs == 2 && over_soft == 1) ? EXIT_SUCCESS : EXIT_FAILURE;
}
