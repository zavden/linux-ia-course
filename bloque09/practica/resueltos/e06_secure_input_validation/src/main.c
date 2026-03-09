#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Este ejercicio muestra una idea clave de seguridad:
 * nunca confiar en input externo (CLI, archivos, red, etc.).
 *
 * Regla de oro:
 * 1) validar formato,
 * 2) validar rango,
 * 3) solo entonces usar el dato.
 */

/* Caracteres permitidos para usuario local simple. */
static int is_valid_user_char(unsigned char c) {
    return (isalnum(c) != 0) || c == '_' || c == '-' || c == '.';
}

/*
 * Politica basica para username:
 * - longitud minima: 3
 * - longitud maxima: 32
 * - charset acotado
 */
static int validate_username(const char *user) {
    if (user == NULL) {
        return 0;
    }

    size_t len = strlen(user);
    if (len < 3U || len > 32U) {
        return 0;
    }

    for (size_t i = 0; i < len; ++i) {
        if (!is_valid_user_char((unsigned char)user[i])) {
            return 0;
        }
    }

    return 1;
}

/*
 * Conversion estricta de puerto:
 * - sin signos + o -
 * - sin basura al final
 * - rango valido 1..65535
 */
static int parse_port_strict(const char *text, unsigned *out_port) {
    if (text == NULL || out_port == NULL || text[0] == '\0') {
        return -1;
    }

    /* Bloqueamos entradas tipo "+80" o "-1" por claridad de politica. */
    if (text[0] == '+' || text[0] == '-') {
        return -1;
    }

    errno = 0;
    char *end = NULL;
    unsigned long value = strtoul(text, &end, 10);

    /* Error de conversion, overflow o input vacio. */
    if (errno != 0 || end == text) {
        return -1;
    }

    /* Cadena debe quedar consumida por completo: "443abc" es invalido. */
    if (*end != '\0') {
        return -1;
    }

    if (value == 0UL || value > 65535UL) {
        return -1;
    }

    *out_port = (unsigned)value;
    return 0;
}

int main(int argc, char **argv) {
    /* Defaults para ejecucion rapida del ejercicio. */
    const char *user = "alice_01";
    const char *port_text = "443";

    if (argc == 3) {
        user = argv[1];
        port_text = argv[2];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [<usuario> <puerto>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Validaciones independientes para poder reportar ambas señales. */
    int user_ok = validate_username(user);

    unsigned port = 0U;
    int port_ok = (parse_port_strict(port_text, &port) == 0) ? 1 : 0;

    /*
     * Salida estable para test automatizado y observabilidad.
     * Si puerto invalido, mostramos 0 para dejar claro que no se acepta.
     */
    printf("user=%s user_ok=%d port_ok=%d port=%u\n", user, user_ok, port_ok, port_ok ? port : 0U);

    return (user_ok == 1 && port_ok == 1) ? EXIT_SUCCESS : EXIT_FAILURE;
}
