#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Formato esperado por linea:
 * user|salt_hex|mac_hex|flags
 *
 * Ejemplo:
 * alice|0011223344556677|aaaaaaaa...64hex...aaaa|1
 */

static int is_username_char(unsigned char c) {
    return (isalnum(c) != 0) || c == '_' || c == '-' || c == '.';
}

static int is_hex_char(unsigned char c) {
    return isxdigit(c) != 0;
}

static void chomp(char *line) {
    if (line == NULL) {
        return;
    }

    size_t n = strlen(line);
    while (n > 0U && (line[n - 1] == '\n' || line[n - 1] == '\r')) {
        line[--n] = '\0';
    }
}

/* Usuario simple: 3..32 chars con set acotado. */
static int validate_user(const char *user) {
    if (user == NULL) {
        return -1;
    }

    size_t len = strlen(user);
    if (len < 3U || len > 32U) {
        return -1;
    }

    for (size_t i = 0; i < len; ++i) {
        if (!is_username_char((unsigned char)user[i])) {
            return -1;
        }
    }

    return 0;
}

/*
 * Valida cadena hexadecimal con longitud exacta.
 * En minivault de ejemplo usamos salt de 8 bytes => 16 hex chars.
 */
static int validate_hex_exact(const char *hex, size_t expected_len) {
    if (hex == NULL) {
        return -1;
    }

    size_t len = strlen(hex);
    if (len != expected_len) {
        return -1;
    }

    for (size_t i = 0; i < len; ++i) {
        if (!is_hex_char((unsigned char)hex[i])) {
            return -1;
        }
    }

    return 0;
}

/* Flags de ejemplo: rango 0..7 (3 bits de policy local). */
static int parse_flags(const char *text, unsigned *out) {
    if (text == NULL || out == NULL || text[0] == '\0') {
        return -1;
    }

    if (text[0] == '+' || text[0] == '-') {
        return -1;
    }

    errno = 0;
    char *end = NULL;
    unsigned long v = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0') {
        return -1;
    }

    if (v > 7UL) {
        return -1;
    }

    *out = (unsigned)v;
    return 0;
}

/*
 * Parsea una linea y reporta si es admin via bit0 de flags.
 * Devuelve 0 si el registro es valido.
 */
static int parse_record_line(const char *line, int *out_admin) {
    if (line == NULL || out_admin == NULL) {
        return -1;
    }

    char copy[1024];
    if (snprintf(copy, sizeof(copy), "%s", line) >= (int)sizeof(copy)) {
        return -1;
    }

    char *fields[4] = {0};
    size_t nf = 0U;

    char *save = NULL;
    char *tok = strtok_r(copy, "|", &save);
    while (tok != NULL) {
        if (nf >= 4U) {
            return -1;
        }
        fields[nf++] = tok;
        tok = strtok_r(NULL, "|", &save);
    }

    if (nf != 4U) {
        return -1;
    }

    if (validate_user(fields[0]) != 0) {
        return -1;
    }

    if (validate_hex_exact(fields[1], 16U) != 0) {
        return -1;
    }

    /* HMAC-SHA256 en hex: 32 bytes => 64 caracteres hex. */
    if (validate_hex_exact(fields[2], 64U) != 0) {
        return -1;
    }

    unsigned flags = 0U;
    if (parse_flags(fields[3], &flags) != 0) {
        return -1;
    }

    *out_admin = (flags & 0x1U) ? 1 : 0;
    return 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/records.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<archivo_registros>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    int valid = 0;
    int invalid = 0;
    int admin = 0;

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        chomp(line);

        /* Ignoramos comentarios y lineas vacias del fixture. */
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }

        int is_admin = 0;
        if (parse_record_line(line, &is_admin) == 0) {
            valid++;
            admin += is_admin;
        } else {
            invalid++;
        }
    }

    fclose(f);

    printf("valid=%d invalid=%d admin=%d\n", valid, invalid, admin);

    /* En modo default verificamos resultado esperado del fixture del curso. */
    if (argc == 1) {
        return (valid == 3 && invalid == 1 && admin == 2) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
