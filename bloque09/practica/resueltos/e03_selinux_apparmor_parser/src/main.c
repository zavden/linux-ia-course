#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Parsea modo SELinux desde salida textual de sestatus. */
static int parse_selinux_mode(const char *path, char *out_mode, size_t out_sz) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char line[256];
    while (fgets(line, sizeof(line), f) != NULL) {
        if (strncmp(line, "Current mode:", 13) == 0) {
            char mode[64] = {0};
            if (sscanf(line, "Current mode: %63s", mode) == 1) {
                snprintf(out_mode, out_sz, "%s", mode);
                fclose(f);
                return 0;
            }
        }
    }

    fclose(f);
    return -1;
}

/*
 * Parsea conteos AppArmor enforce/complain desde aa-status.
 * Usamos estrategia tolerante (strstr + parseo del entero inicial)
 * porque la salida puede variar levemente entre distribuciones.
 */
static int parse_apparmor_counts(const char *path, int *out_enforce, int *out_complain) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    int enf = 0;
    int comp = 0;

    char line[256];
    while (fgets(line, sizeof(line), f) != NULL) {
        char *end = NULL;
        long n = strtol(line, &end, 10);

        /* Si la linea no empieza con entero, no nos sirve para conteos. */
        if (end == line) {
            continue;
        }

        if (strstr(line, "profiles are in enforce mode") != NULL) {
            enf = (int)n;
            continue;
        }
        if (strstr(line, "profiles are in complain mode") != NULL) {
            comp = (int)n;
            continue;
        }
    }

    fclose(f);
    *out_enforce = enf;
    *out_complain = comp;
    return 0;
}

int main(int argc, char **argv) {
    const char *selinux_path = (argc >= 2) ? argv[1] : "tests/data/sestatus.sample";
    const char *aa_path = (argc >= 3) ? argv[2] : "tests/data/aa-status.sample";

    char sel_mode[64] = {0};
    int aa_enforce = 0;
    int aa_complain = 0;

    if (parse_selinux_mode(selinux_path, sel_mode, sizeof(sel_mode)) == -1) {
        perror("parse_selinux_mode");
        return EXIT_FAILURE;
    }

    if (parse_apparmor_counts(aa_path, &aa_enforce, &aa_complain) == -1) {
        perror("parse_apparmor_counts");
        return EXIT_FAILURE;
    }

    printf("selinux_mode=%s aa_enforce=%d aa_complain=%d\n", sel_mode, aa_enforce, aa_complain);

    return (strcmp(sel_mode, "enforcing") == 0 && aa_enforce == 2 && aa_complain == 1)
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
