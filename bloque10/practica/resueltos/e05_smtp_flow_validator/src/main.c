#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Estados del flujo SMTP simplificado para laboratorio.
 */
enum {
    ST_GREET = 0,
    ST_MAIL = 1,
    ST_RCPT_OR_DATA = 2,
    ST_DATA_BODY = 3,
    ST_QUIT = 4,
    ST_DONE = 5,
};

static void rstrip(char *s) {
    size_t n = strlen(s);
    while (n > 0U && (s[n - 1U] == '\n' || s[n - 1U] == '\r')) {
        s[--n] = '\0';
    }
}

/* Ignora espacios de la izquierda para parseo robusto. */
static char *lskip(char *s) {
    while (*s != '\0' && isspace((unsigned char)*s)) s++;
    return s;
}

/* Comparacion case-insensitive sobre prefijo ASCII. */
static int starts_with_ci(const char *s, const char *prefix) {
    for (size_t i = 0; prefix[i] != '\0'; ++i) {
        if (s[i] == '\0') return 0;
        if (tolower((unsigned char)s[i]) != tolower((unsigned char)prefix[i])) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/smtp.good.session";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<smtp_script>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    int state = ST_GREET;
    int valid = 1;
    int rcpt_count = 0;
    int body_lines = 0;

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        rstrip(line);
        char *p = lskip(line);

        /* Permite lineas vacias fuera de cuerpo sin afectar flujo. */
        if (p[0] == '\0' && state != ST_DATA_BODY) {
            continue;
        }

        if (state == ST_GREET) {
            if (starts_with_ci(p, "EHLO ") || starts_with_ci(p, "HELO ")) {
                state = ST_MAIL;
            } else {
                valid = 0;
                break;
            }
            continue;
        }

        if (state == ST_MAIL) {
            if (starts_with_ci(p, "MAIL FROM:")) {
                state = ST_RCPT_OR_DATA;
            } else {
                valid = 0;
                break;
            }
            continue;
        }

        if (state == ST_RCPT_OR_DATA) {
            if (starts_with_ci(p, "RCPT TO:")) {
                rcpt_count++;
                continue;
            }

            if (starts_with_ci(p, "DATA")) {
                if (rcpt_count <= 0) {
                    valid = 0;
                    break;
                }
                state = ST_DATA_BODY;
                continue;
            }

            valid = 0;
            break;
        }

        if (state == ST_DATA_BODY) {
            if (strcmp(p, ".") == 0) {
                state = ST_QUIT;
            } else {
                /* Contamos inclusive lineas vacias del cuerpo. */
                body_lines++;
            }
            continue;
        }

        if (state == ST_QUIT) {
            if (starts_with_ci(p, "QUIT")) {
                state = ST_DONE;
            } else {
                valid = 0;
            }
            break;
        }
    }

    fclose(f);

    if (state != ST_DONE) {
        valid = 0;
    }

    printf("valid=%d rcpt=%d body_lines=%d\n", valid, rcpt_count, body_lines);

    return valid ? EXIT_SUCCESS : EXIT_FAILURE;
}
