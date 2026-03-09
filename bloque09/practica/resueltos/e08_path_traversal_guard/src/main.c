#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Problema de seguridad:
 * si aceptas "../../etc/shadow" y concatenas directo con raiz,
 * un atacante puede escapar del sandbox logico.
 *
 * Este ejercicio implementa una normalizacion controlada.
 */

/* Charset conservador para nombres de archivo en este ejercicio. */
static int is_safe_segment_char(unsigned char c) {
    return (isalnum(c) != 0) || c == '_' || c == '-' || c == '.';
}

static int validate_segment(const char *seg) {
    if (seg == NULL || seg[0] == '\0') {
        return -1;
    }

    for (size_t i = 0; seg[i] != '\0'; ++i) {
        if (!is_safe_segment_char((unsigned char)seg[i])) {
            return -1;
        }
    }

    return 0;
}

/*
 * Normaliza una ruta relativa:
 * - ignora "."
 * - procesa ".." con pila
 * - rechaza escape por encima de raiz
 * - rechaza ruta absoluta
 */
static int normalize_relative_path(const char *input, char *out, size_t out_sz) {
    if (input == NULL || out == NULL || out_sz == 0U) {
        return -1;
    }

    if (input[0] == '/' || input[0] == '\0') {
        return -1;
    }

    char work[512];
    if (snprintf(work, sizeof(work), "%s", input) >= (int)sizeof(work)) {
        return -1;
    }

    char *stack[128];
    size_t depth = 0U;

    char *save = NULL;
    char *tok = strtok_r(work, "/", &save);
    while (tok != NULL) {
        if (strcmp(tok, ".") == 0 || tok[0] == '\0') {
            tok = strtok_r(NULL, "/", &save);
            continue;
        }

        if (strcmp(tok, "..") == 0) {
            if (depth == 0U) {
                return -1;
            }
            depth--;
            tok = strtok_r(NULL, "/", &save);
            continue;
        }

        if (validate_segment(tok) != 0) {
            return -1;
        }

        if (depth >= (sizeof(stack) / sizeof(stack[0]))) {
            return -1;
        }

        stack[depth++] = tok;
        tok = strtok_r(NULL, "/", &save);
    }

    if (depth == 0U) {
        if (out_sz < 2U) {
            return -1;
        }
        out[0] = '.';
        out[1] = '\0';
        return 0;
    }

    size_t used = 0U;
    out[0] = '\0';

    for (size_t i = 0; i < depth; ++i) {
        size_t seg_len = strlen(stack[i]);

        if (i > 0U) {
            if (used + 1U >= out_sz) {
                return -1;
            }
            out[used++] = '/';
            out[used] = '\0';
        }

        if (used + seg_len >= out_sz) {
            return -1;
        }

        memcpy(out + used, stack[i], seg_len);
        used += seg_len;
        out[used] = '\0';
    }

    return 0;
}

int main(int argc, char **argv) {
    const char *root = "/srv/minivault";
    const char *candidate = "docs/report.txt";

    if (argc == 3) {
        root = argv[1];
        candidate = argv[2];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [<root_absoluto> <ruta_relativa>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Root debe ser absoluto para politica clara. */
    if (root[0] != '/') {
        printf("candidate=%s allowed=0 reason=bad_root\n", candidate);
        return EXIT_FAILURE;
    }

    char normalized[512];
    if (normalize_relative_path(candidate, normalized, sizeof(normalized)) != 0) {
        printf("candidate=%s allowed=0 reason=bad_path\n", candidate);
        return EXIT_FAILURE;
    }

    char full[1024];
    if (snprintf(full, sizeof(full), "%s/%s", root, normalized) >= (int)sizeof(full)) {
        printf("candidate=%s allowed=0 reason=too_long\n", candidate);
        return EXIT_FAILURE;
    }

    printf("candidate=%s allowed=1 normalized=%s full=%s\n", candidate, normalized, full);
    return EXIT_SUCCESS;
}
