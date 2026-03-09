#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Bits de capabilities usados en el ejercicio. */
enum {
    CAP_NET_BIND_SERVICE = 10,
    CAP_NET_RAW = 13,
};

/*
 * Extrae CapEff de archivo estilo /proc/self/status.
 * Devuelve 0 en éxito y coloca máscara en out_mask.
 */
static int parse_cap_eff(const char *path, uint64_t *out_mask) {
    FILE *f = fopen(path, "r");
    if (!f) {
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), f) != NULL) {
        if (strncmp(line, "CapEff:", 7) == 0) {
            char hex[64] = {0};
            if (sscanf(line, "CapEff:\t%63s", hex) != 1) {
                fclose(f);
                return -1;
            }

            *out_mask = strtoull(hex, NULL, 16);
            fclose(f);
            return 0;
        }
    }

    fclose(f);
    return -1;
}

static int has_cap(uint64_t mask, int cap_bit) {
    return (mask & (1ULL << cap_bit)) ? 1 : 0;
}

int main(int argc, char **argv) {
    const char *path = (argc >= 2) ? argv[1] : "tests/data/status.sample";

    uint64_t cap_eff = 0;
    if (parse_cap_eff(path, &cap_eff) == -1) {
        perror("parse_cap_eff");
        return EXIT_FAILURE;
    }

    int bind = has_cap(cap_eff, CAP_NET_BIND_SERVICE);
    int raw = has_cap(cap_eff, CAP_NET_RAW);

    printf("cap_eff=0x%llx bind=%d raw=%d\n", (unsigned long long)cap_eff, bind, raw);

    return (bind == 1 && raw == 1) ? EXIT_SUCCESS : EXIT_FAILURE;
}
