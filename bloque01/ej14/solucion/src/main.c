/*
 * Ejercicio 1.4 — Manejo de Errores POSIX (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Las macros de preprocesador son herramientas poderosísimas en C. Nos permiten 
 * inspeccionar metadatos mágicos del compilador como __FILE__ (nombre de este) 
 * y __LINE__ (línea exacta donde se invocó). 
 * Además, demostramos el patrón aceptado internacionalmente "goto cleanup" 
 * implementado incluso en el mismísimo kernel de Linux.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/*
 * LA MACRO CHECK
 * Evaluamos (call). Si es NULL o < 0 (depende de la convención de la API), aborta.
 * Al usar un bloque `do { ... } while(0)`, nos aseguramos de que esta macro 
 * pueda usarse limpiamente dentro de un if unilineal sin romper la semántica de C.
 */
#define CHECK_PTR(ptr_call) \
    do { \
        if ((ptr_call) == NULL) { \
            fprintf(stderr, "[ERROR] %s:%d - %s\n", __FILE__, __LINE__, #ptr_call); \
            perror("Motivo"); \
            goto cleanup; \
        } \
    } while(0)

int main(void) {
    /* 
     * SETUP SEGURO
     * Todo puntero a recurso externo o RAM se inicializa SIEMPRE en NULL.
     * Esto hace que nuestro `cleanup` sepa exactamente qué destruir y qué ignorar.
     */
    FILE *f = NULL;
    char *buffer = NULL;
    int error_status = EXIT_FAILURE; // Pesimistas: asumen que todo va a fallar.

    printf("Iniciando transaccion compleja...\n");

    // 1. Intentamos obtener RAM
    buffer = malloc(1024);
    CHECK_PTR(buffer); // Evalúa. Si da NULL, salta e imprime la línea exacta.

    // 2. Intentamos abrir el archivo fantasma
    // Esta llamada VA A FALLAR y va a hacer salatar el goto de CHECK_PTR
    f = fopen("archivo_fake.txt", "r");
    CHECK_PTR(f); 

    // --- Si algo fallase arriba, nunca se ejecutaría de aquí en adelante ---

    printf("Transaccion exitosa. Trabajando...\n");
    error_status = EXIT_SUCCESS; // Todo salió bien, marcamos éxito.

cleanup:
    /*
     * LA SECCIÓN DE CLEANUP UNIFICADA
     * Cualquier punto de retorno de esta función pasa por aquí obligatoriamente.
     * Es CÓDIGO IDEMPOTENTE: no asume que las cosas están abiertas, lo comprueba.
     */
    printf("Iniciando cleanup unificado...\n");
    
    if (f != NULL) {
        printf(" -> Cerrando archivo abierto.\n");
        fclose(f);
    }
    
    if (buffer != NULL) {
        printf(" -> Liberando 1024 bytes de RAM protegida.\n");
        free(buffer);
    }

    // Devolvemos lo que hayamos logrado marcar en error_status
    if (error_status == EXIT_FAILURE) {
        printf("Finalizo con Error (Controlado limpia y seguramente).\n");
    }
    
    return error_status;
}
