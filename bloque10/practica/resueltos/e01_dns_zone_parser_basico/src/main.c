#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int total;
    int a;
    int aaaa;
    int cname;
    int mx;
    int ns;
    int soa;
    int txt;
} zone_stats_t;

/*
 * Recorta espacios al inicio y fin para simplificar parseo.
 */
static char *trim(char *s) {
    if (s == NULL) return NULL;

    while (*s != '\0' && isspace((unsigned char)*s)) {
        s++;
    }

    if (*s == '\0') {
        return s;
    }

    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }

    return s;
}

/*
 * Elimina comentario ';' cuando no estamos dentro de comillas.
 * Esto evita cortar TXT validos que podrian contener ';'.
 */
static void strip_dns_comment(char *line) {
    int in_quotes = 0;

    for (size_t i = 0; line[i] != '\0'; ++i) {
        if (line[i] == '"') {
            in_quotes = !in_quotes;
            continue;
        }

        if (!in_quotes && line[i] == ';') {
            line[i] = '\0';
            return;
        }
    }
}

/*
 * Busca el tipo de registro dentro de tokens de la linea.
 * No asumimos posicion fija porque en zonas reales puede variar TTL/CLASS.
 */
static const char *detect_record_type(char *tokens[], size_t ntok) {
    for (size_t i = 0; i < ntok; ++i) {
        if (strcmp(tokens[i], "A") == 0) return "A";
        if (strcmp(tokens[i], "AAAA") == 0) return "AAAA";
        if (strcmp(tokens[i], "CNAME") == 0) return "CNAME";
        if (strcmp(tokens[i], "MX") == 0) return "MX";
        if (strcmp(tokens[i], "NS") == 0) return "NS";
        if (strcmp(tokens[i], "SOA") == 0) return "SOA";
        if (strcmp(tokens[i], "TXT") == 0) return "TXT";
    }

    return NULL;
}

static int parse_zone_file(const char *path, zone_stats_t *out) {
    if (path == NULL || out == NULL) return -1;

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    memset(out, 0, sizeof(*out));

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        /* Limpiamos salto de linea para trabajar con string estable. */
        line[strcspn(line, "\r\n")] = '\0';
        strip_dns_comment(line);

        char *p = trim(line);
        if (*p == '\0') {
            continue;
        }

        /* Directivas de zona como $ORIGIN/$TTL no son RR de datos. */
        if (*p == '$') {
            continue;
        }

        char *tokens[32];
        size_t ntok = 0U;

        char *tok = strtok(p, " \t");
        while (tok != NULL) {
            if (ntok >= (sizeof(tokens) / sizeof(tokens[0]))) {
                fclose(f);
                return -1;
            }
            tokens[ntok++] = tok;
            tok = strtok(NULL, " \t");
        }

        if (ntok == 0U) {
            continue;
        }

        const char *type = detect_record_type(tokens, ntok);
        if (type == NULL) {
            continue;
        }

        out->total++;
        if (strcmp(type, "A") == 0) out->a++;
        else if (strcmp(type, "AAAA") == 0) out->aaaa++;
        else if (strcmp(type, "CNAME") == 0) out->cname++;
        else if (strcmp(type, "MX") == 0) out->mx++;
        else if (strcmp(type, "NS") == 0) out->ns++;
        else if (strcmp(type, "SOA") == 0) out->soa++;
        else if (strcmp(type, "TXT") == 0) out->txt++;
    }

    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/zone.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<zone_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    zone_stats_t st;
    if (parse_zone_file(path, &st) != 0) {
        perror("parse_zone_file");
        return EXIT_FAILURE;
    }

    printf("total=%d A=%d AAAA=%d CNAME=%d MX=%d NS=%d SOA=%d TXT=%d\n",
           st.total, st.a, st.aaaa, st.cname, st.mx, st.ns, st.soa, st.txt);

    if (argc == 1) {
        return (st.total == 9 && st.a == 2 && st.aaaa == 1 && st.cname == 1 &&
                st.mx == 1 && st.ns == 2 && st.soa == 1 && st.txt == 1)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
