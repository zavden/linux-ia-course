#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Convierte tamaño textual como 20.00g / 512m a MiB aproximados. */
static long size_to_mib(const char *s) {
    double v = 0.0;
    char unit = 'm';

    if (sscanf(s, "%lf%c", &v, &unit) != 2) {
        return -1;
    }

    unit = (char)tolower((unsigned char)unit);
    if (unit == 'g') {
        return (long)(v * 1024.0);
    }
    if (unit == 'm') {
        return (long)v;
    }
    if (unit == 't') {
        return (long)(v * 1024.0 * 1024.0);
    }
    return -1;
}

int main(int argc, char **argv) {
    /* Archivo estilo lvs con separador ';' (ideal para parse robusto). */
    const char *path = (argc >= 2) ? argv[1] : "tests/data/lvs.sample";

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen lvs");
        return EXIT_FAILURE;
    }

    int lvs_count = 0;
    int thin_count = 0;
    long total_mib = 0;

    char line[256];
    while (fgets(line, sizeof(line), f) != NULL) {
        /* Campos clave de lvs: LV, VG, Attr, Size. */
        char lv[64] = {0};
        char vg[64] = {0};
        char attr[32] = {0};
        char size[32] = {0};

        if (sscanf(line, "%63[^;];%63[^;];%31[^;];%31s", lv, vg, attr, size) != 4) {
            continue;
        }
        (void)lv;
        (void)vg;

        lvs_count++;
        /* Thin/snapshot suelen marcarse con 't' en el atributo. */
        if (strchr(attr, 't') != NULL) {
            thin_count++;
        }

        long mib = size_to_mib(size);
        if (mib > 0) {
            total_mib += mib;
        }
    }

    fclose(f);

    /* Resumen final útil para inventario o dashboards. */
    printf("lvs=%d thin=%d total_mib=%ld\n", lvs_count, thin_count, total_mib);

    return (lvs_count == 4 && thin_count == 1 && total_mib > 10000) ? EXIT_SUCCESS : EXIT_FAILURE;
}
