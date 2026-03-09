#include <stdio.h>
#include <stdlib.h>

/*
 * Reto C02:
 * gateway de autenticacion con PAM y politicas defensivas.
 *
 * Enfasis:
 * - manejo correcto de errores de autenticacion
 * - endurecimiento frente a fuerza bruta
 * - trazabilidad de intentos
 */

/* TODO 1: definir interfaz auth_provider para permitir mock en tests. */

/* TODO 2: integrar flujo PAM (start/authenticate/acct_mgmt/end). */

/* TODO 3: agregar rate-limit por usuario/ip y ventana temporal configurable. */

/* TODO 4: implementar lockout temporal con contador persistente seguro. */

/* TODO 5: emitir reporte final sin filtrar informacion util para atacantes. */

int main(void) {
    puts("C02 plantilla lista");
    return EXIT_SUCCESS;
}
