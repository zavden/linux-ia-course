#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Devuelve 1 si la línea es comentario/blank y debe ignorarse.
 */
static int is_ignorable_line(const char *line) {
    size_t i = 0;
    while (line[i] != '\0' && isspace((unsigned char)line[i])) {
        i++;
    }
    return (line[i] == '\0' || line[i] == '#');
}

/*
 * Parser simple de archivo con formato "spec mountpoint fstype ...".
 * Cuenta líneas válidas y guarda el primer mountpoint encontrado.
 */
static int parse_table(const char *path, int *out_count, char *out_first, size_t out_first_sz) {
    FILE *f = fopen(path, "r");
    if (!f) {
        return -1;
    }

    char line[512];
    int count = 0;
    out_first[0] = '\0';

    while (fgets(line, sizeof(line), f) != NULL) {
        if (is_ignorable_line(line)) {
            continue;
        }

        char spec[128] = {0};
        char mnt[128] = {0};
        char fstype[64] = {0};

        if (sscanf(line, "%127s %127s %63s", spec, mnt, fstype) < 3) {
            continue;
        }

        if (count == 0) {
            snprintf(out_first, out_first_sz, "%s", mnt);
        }

        count++;
    }

    fclose(f);
    *out_count = count;
    return 0;
}

int main(int argc, char **argv) {
    /* Permite usar archivos reales o fixtures de prueba. */
    const char *mounts = (argc >= 2) ? argv[1] : "/proc/mounts";
    const char *fstab = (argc >= 3) ? argv[2] : "/etc/fstab";

    int mounts_count = 0;
    int fstab_count = 0;
    char first_mounts[128];
    char first_fstab[128];

    if (parse_table(mounts, &mounts_count, first_mounts, sizeof(first_mounts)) == -1) {
        perror("parse mounts");
        return EXIT_FAILURE;
    }

    if (parse_table(fstab, &fstab_count, first_fstab, sizeof(first_fstab)) == -1) {
        perror("parse fstab");
        return EXIT_FAILURE;
    }

    /* Comparativo mínimo entre estado activo (mounts) y declarado (fstab). */
    printf("mounts=%d fstab=%d first_mount=%s first_fstab=%s\n",
           mounts_count,
           fstab_count,
           first_mounts,
           first_fstab);

    return (mounts_count > 0 && fstab_count > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
