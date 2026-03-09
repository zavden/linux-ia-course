#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Lee archivo completo en memoria para un lint rápido por patrones.
 * Para práctica inicial evitamos parser JSON completo y nos concentramos
 * en reglas de seguridad operativa.
 */
static char *read_all(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }

    long sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return NULL;
    }

    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }

    char *buf = (char *)malloc((size_t)sz + 1U);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t rd = fread(buf, 1U, (size_t)sz, f);
    fclose(f);

    if (rd != (size_t)sz) {
        free(buf);
        return NULL;
    }

    buf[rd] = '\0';
    return buf;
}

/* Busca token JSON simple: "token" */
static int has_json_token(const char *json, const char *token) {
    char needle[128];
    if (snprintf(needle, sizeof(needle), "\"%s\"", token) >= (int)sizeof(needle)) {
        return 0;
    }
    return (strstr(json, needle) != NULL) ? 1 : 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/seccomp.good.json";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<seccomp_profile_json>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *json = read_all(path);
    if (!json) {
        perror("read_all");
        return EXIT_FAILURE;
    }

    /* Regla 1: política por defecto debe negar o errar (no allow-all). */
    int default_ok = (strstr(json, "\"defaultAction\"") != NULL &&
                      strstr(json, "SCMP_ACT_ERRNO") != NULL)
                         ? 1
                         : 0;

    /* Regla 2: syscalls base necesarias para proceso mínimo en userspace. */
    int required_ok = has_json_token(json, "read") &&
                      has_json_token(json, "write") &&
                      has_json_token(json, "exit") &&
                      has_json_token(json, "rt_sigreturn");

    /* Regla 3: syscalls de alto riesgo que no deberían estar en allow-list básica. */
    int dangerous = has_json_token(json, "ptrace") ||
                    has_json_token(json, "kexec_load") ||
                    has_json_token(json, "keyctl");

    int secure = (default_ok && required_ok && !dangerous) ? 1 : 0;

    printf("default_ok=%d required_ok=%d dangerous=%d secure=%d\n",
           default_ok, required_ok ? 1 : 0, dangerous ? 1 : 0, secure);

    free(json);

    return secure ? EXIT_SUCCESS : EXIT_FAILURE;
}
