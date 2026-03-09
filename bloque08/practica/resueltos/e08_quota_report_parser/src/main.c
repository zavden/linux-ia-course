#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

static int is_comment_or_blank(const char *line) {
    size_t i = 0;
    while (line[i] != '\0' && isspace((unsigned char)line[i])) {
        i++;
    }
    return (line[i] == '\0' || line[i] == '#');
}

int main(int argc, char **argv) {
    /* Reporte fuente configurable para pruebas y datos reales. */
    const char *path = (argc >= 2) ? argv[1] : "tests/data/quota.sample";

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen quota");
        return EXIT_FAILURE;
    }

    int users = 0;
    int over_soft = 0;
    int over_hard = 0;

    char line[256];
    while (fgets(line, sizeof(line), f) != NULL) {
        /* Omitimos headers/comentarios para parse limpio. */
        if (is_comment_or_blank(line)) {
            continue;
        }

        char user[64] = {0};
        long blocks = 0;
        long soft = 0;
        long hard = 0;

        if (sscanf(line, "%63s %ld %ld %ld", user, &blocks, &soft, &hard) != 4) {
            continue;
        }
        (void)user;

        users++;
        /* Riesgo operativo: uso por encima del umbral soft. */
        if (blocks > soft) {
            over_soft++;
        }
        /* Riesgo crítico: excede hard limit. */
        if (blocks > hard) {
            over_hard++;
        }
    }

    fclose(f);

    /* Métricas mínimas para auditoría de cuotas. */
    printf("users=%d over_soft=%d over_hard=%d\n", users, over_soft, over_hard);

    return (users == 4 && over_soft == 2 && over_hard == 2) ? EXIT_SUCCESS : EXIT_FAILURE;
}
