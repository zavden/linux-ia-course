#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int code;
    int cont;
    const char *text;
} smtp_reply_t;

static void rstrip(char *s) {
    size_t n = strlen(s);
    while (n > 0U && (s[n - 1U] == '\n' || s[n - 1U] == '\r')) {
        s[--n] = '\0';
    }
}

/*
 * Parsea linea estilo SMTP:
 * 250-PIPELINING
 * 250 OK
 */
static int parse_reply_line(char *line, smtp_reply_t *out) {
    if (line == NULL || out == NULL) return -1;

    if (!isdigit((unsigned char)line[0]) ||
        !isdigit((unsigned char)line[1]) ||
        !isdigit((unsigned char)line[2])) {
        return -1;
    }

    char sep = line[3];
    if (sep != '-' && sep != ' ') {
        return -1;
    }

    errno = 0;
    char code_s[4];
    memcpy(code_s, line, 3);
    code_s[3] = '\0';

    char *end = NULL;
    long code = strtol(code_s, &end, 10);
    if (errno != 0 || end == code_s || *end != '\0' || code < 100 || code > 599) {
        return -1;
    }

    out->code = (int)code;
    out->cont = (sep == '-') ? 1 : 0;
    out->text = line + 4;
    return 0;
}

/* Contiene token ignorando mayus/minus para texto ASCII simple. */
static int contains_ci(const char *haystack, const char *needle) {
    size_t nlen = strlen(needle);
    if (nlen == 0U) return 1;

    for (size_t i = 0; haystack[i] != '\0'; ++i) {
        size_t j = 0U;
        while (needle[j] != '\0' && haystack[i + j] != '\0' &&
               tolower((unsigned char)haystack[i + j]) == tolower((unsigned char)needle[j])) {
            j++;
        }
        if (needle[j] == '\0') {
            return 1;
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/smtp.replies.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<smtp_replies_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    int greet = 0;
    int final = 0;
    int caps = 0;
    int starttls = 0;

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        rstrip(line);
        if (line[0] == '\0') {
            continue;
        }

        smtp_reply_t rep;
        if (parse_reply_line(line, &rep) != 0) {
            continue;
        }

        if (greet == 0 && rep.code == 220) {
            greet = rep.code;
        }

        if (rep.code == 250) {
            final = 250;

            if (rep.cont) {
                caps++;
            }

            if (contains_ci(rep.text, "STARTTLS")) {
                starttls = 1;
            }
        }
    }

    fclose(f);

    printf("greet=%d final=%d caps=%d starttls=%d\n", greet, final, caps, starttls);

    if (argc == 1) {
        return (greet == 220 && final == 250 && caps == 2 && starttls == 1)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return (final == 250) ? EXIT_SUCCESS : EXIT_FAILURE;
}
