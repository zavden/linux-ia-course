#include <stdio.h>
#include <stdlib.h>

/*
 * Reto C01:
 * crear una herramienta CLI realista para guardar/leer secretos.
 *
 * Enfasis:
 * - validacion estricta de input
 * - integridad criptografica
 * - minima superficie de privilegio
 */

/* TODO 1: definir modelo de datos para registros de vault (user, salt, mac, flags, payload). */

/* TODO 2: implementar parser robusto con validaciones de longitud y formato. */

/* TODO 3: integrar funciones crypto (hash/hmac) para detectar manipulaciones del archivo. */

/* TODO 4: agregar comandos CLI: add, get, list, verify, rotate-key. */

/* TODO 5: registrar eventos de seguridad (fallos de parse, acceso denegado, integridad rota). */

int main(void) {
    puts("C01 plantilla lista");
    return EXIT_SUCCESS;
}
