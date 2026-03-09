#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>

/*
 * Reto C02:
 * miniserver HTTP 1.1 con workers persistentes.
 */

/* TODO 1: crear socket servidor y preparar listen en puerto configurable */

/* TODO 2: diseñar cola de client_fd con mutex + condvar */

/* TODO 3: worker: leer request line, validar método/ruta y responder */

/* TODO 4: implementar shutdown gracioso (SIGINT) y join de workers */

/* TODO 5: agregar logs de requests, status code y tiempo de servicio */

int main(void) {
    puts("C02 plantilla lista");
    return EXIT_SUCCESS;
}
