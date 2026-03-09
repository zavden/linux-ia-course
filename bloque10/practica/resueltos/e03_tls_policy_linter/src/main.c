#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int protocols_ok;
    int ciphers_ok;
    int rsa_ok;
} tls_checks_t;

static void trim_inplace(char *s) {
    if (s == NULL || s[0] == '\0') return;

    char *start = s;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }

    char *end = start + strlen(start);
    while (end > start && isspace((unsigned char)end[-1])) {
        end[-1] = '\0';
        end--;
    }

    if (start != s) {
        memmove(s, start, strlen(start) + 1U);
    }
}

/*
 * Busca substring "token" delimitado por separadores comunes (, : espacio).
 */
static int contains_tokenish(const char *text, const char *token) {
    const size_t tlen = strlen(token);
    const char *p = text;

    while ((p = strstr(p, token)) != NULL) {
        char left = (p == text) ? '\0' : p[-1];
        char right = p[tlen];

        int left_ok = (left == '\0' || left == ',' || left == ':' || isspace((unsigned char)left));
        int right_ok = (right == '\0' || right == ',' || right == ':' || isspace((unsigned char)right));

        if (left_ok && right_ok) {
            return 1;
        }

        p += tlen;
    }

    return 0;
}

static int lint_tls_policy(const char *path, tls_checks_t *out) {
    if (path == NULL || out == NULL) return -1;

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    out->protocols_ok = 0;
    out->ciphers_ok = 0;
    out->rsa_ok = 0;

    int saw_protocols = 0;
    int saw_ciphers = 0;
    int saw_rsa = 0;

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';

        char *comment = strchr(line, '#');
        if (comment) {
            *comment = '\0';
        }

        trim_inplace(line);
        if (line[0] == '\0') {
            continue;
        }

        char *eq = strchr(line, '=');
        if (eq == NULL) {
            continue;
        }

        *eq = '\0';
        char *key = line;
        char *val = eq + 1;
        trim_inplace(key);
        trim_inplace(val);

        if (strcmp(key, "protocols") == 0) {
            saw_protocols = 1;

            int has_good = contains_tokenish(val, "TLSv1.2") || contains_tokenish(val, "TLSv1.3");
            int has_bad = contains_tokenish(val, "TLSv1.0") || contains_tokenish(val, "TLSv1.1") ||
                          contains_tokenish(val, "SSLv3") || contains_tokenish(val, "SSLv2");

            out->protocols_ok = (has_good && !has_bad) ? 1 : 0;
            continue;
        }

        if (strcmp(key, "ciphers") == 0) {
            saw_ciphers = 1;

            /* Bloqueamos familias criptograficas obsoletas o peligrosas. */
            int bad = (strstr(val, "RC4") != NULL) ||
                      (strstr(val, "3DES") != NULL) ||
                      (strstr(val, "NULL") != NULL) ||
                      (strstr(val, "MD5") != NULL);

            out->ciphers_ok = bad ? 0 : 1;
            continue;
        }

        if (strcmp(key, "min_rsa_bits") == 0) {
            saw_rsa = 1;

            errno = 0;
            char *end = NULL;
            long bits = strtol(val, &end, 10);
            if (errno == 0 && end != val && *end == '\0' && bits >= 2048) {
                out->rsa_ok = 1;
            } else {
                out->rsa_ok = 0;
            }
            continue;
        }
    }

    fclose(f);

    if (!saw_protocols || !saw_ciphers || !saw_rsa) {
        return -1;
    }

    return 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/tls.good.conf";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<tls_policy_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    tls_checks_t chk;
    if (lint_tls_policy(path, &chk) != 0) {
        perror("lint_tls_policy");
        return EXIT_FAILURE;
    }

    int strong = (chk.protocols_ok && chk.ciphers_ok && chk.rsa_ok) ? 1 : 0;

    printf("protocols_ok=%d ciphers_ok=%d rsa_ok=%d strong=%d\n",
           chk.protocols_ok, chk.ciphers_ok, chk.rsa_ok, strong);

    return strong ? EXIT_SUCCESS : EXIT_FAILURE;
}
