#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>

/*
 * Reto C02:
 * cliente HTTP estilo curl, single-thread y no bloqueante.
 */

/* TODO 1: parsear argumentos: host, puerto, ruta, timeout_ms */

/* TODO 2: resolver DNS con getaddrinfo y crear socket */

/* TODO 3: connect no bloqueante + poll/select + chequeo SO_ERROR */

/* TODO 4: escribir request HTTP/1.1 y leer respuesta por chunks */

/* TODO 5: parsear status line y separar headers/body */

int main(void) {
    puts("C02 plantilla lista");
    return EXIT_SUCCESS;
}
