#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char scheme[16];
    char host[256];
    unsigned port;
    char path[512];
} https_url_t;

/* Hostname conservador para el ejercicio: letras, numeros, punto y guion. */
static int is_valid_host_char(unsigned char c) {
    return (isalnum(c) != 0) || c == '.' || c == '-';
}

static int parse_https_url(const char *url, https_url_t *out) {
    if (url == NULL || out == NULL) return -1;

    const char *sep = strstr(url, "://");
    if (sep == NULL) {
        return -1;
    }

    size_t scheme_len = (size_t)(sep - url);
    if (scheme_len == 0U || scheme_len >= sizeof(out->scheme)) {
        return -1;
    }

    memcpy(out->scheme, url, scheme_len);
    out->scheme[scheme_len] = '\0';

    if (strcmp(out->scheme, "https") != 0) {
        return -1;
    }

    const char *authority = sep + 3;
    const char *path_start = strchr(authority, '/');

    size_t authority_len = path_start ? (size_t)(path_start - authority) : strlen(authority);
    if (authority_len == 0U || authority_len >= 400U) {
        return -1;
    }

    char hostport[400];
    memcpy(hostport, authority, authority_len);
    hostport[authority_len] = '\0';

    /* Separamos host y puerto si aparece ':' en authority. */
    char *colon = strrchr(hostport, ':');
    if (colon != NULL) {
        *colon = '\0';
        const char *port_text = colon + 1;

        if (port_text[0] == '\0') {
            return -1;
        }

        errno = 0;
        char *end = NULL;
        unsigned long p = strtoul(port_text, &end, 10);
        if (errno != 0 || end == port_text || *end != '\0' || p == 0UL || p > 65535UL) {
            return -1;
        }

        out->port = (unsigned)p;
    } else {
        out->port = 443U;
    }

    if (hostport[0] == '\0' || strlen(hostport) >= sizeof(out->host)) {
        return -1;
    }

    for (size_t i = 0; hostport[i] != '\0'; ++i) {
        if (!is_valid_host_char((unsigned char)hostport[i])) {
            return -1;
        }
    }

    snprintf(out->host, sizeof(out->host), "%s", hostport);

    if (path_start == NULL) {
        snprintf(out->path, sizeof(out->path), "/");
    } else {
        if (strlen(path_start) >= sizeof(out->path)) {
            return -1;
        }
        snprintf(out->path, sizeof(out->path), "%s", path_start);
    }

    return 0;
}

int main(int argc, char **argv) {
    const char *url = (argc == 2) ? argv[1] : "https://api.example.com:8443/v1/health";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<https_url>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    https_url_t u;
    if (parse_https_url(url, &u) != 0) {
        printf("url=%s valid=0\n", url);
        return EXIT_FAILURE;
    }

    printf("valid=1 scheme=%s host=%s port=%u path=%s\n",
           u.scheme, u.host, u.port, u.path);

    return EXIT_SUCCESS;
}
