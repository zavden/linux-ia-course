/*
 * Ejercicio 0.5 — Variables de Entorno y Configuración (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Demostrar cómo una aplicación en C debe leer la configuración dinámica
 * inyectada por el entorno (p. ej. Docker Compose, Kubernetes, bash).
 * Se enfatiza la importancia de proveer valores por defecto ("fallbacks")
 * cuando la variable no está definida en el sistema operativo.
 */

#include <stdio.h>  // printf
#include <stdlib.h> // getenv, atoi, EXIT_SUCCESS

int main(void) {
    /*
     * 1. LEER VARIABLES DE ENTORNO
     * getenv("NOMBRE_VARIABLE") busca en la tabla de entorno del proceso
     * que el sistema operativo (p. ej. Linux) le inyectó al arrancar.
     * Retorna un puntero a una cadena estática (char *) o NULL si no existe.
     * 
     * ¡CUIDADO! Nunca intentes modificar la cadena devuelta por getenv().
     * (e.g. port_str[0] = 'a';). Es memoria gestionada por el entorno de C.
     */
    const char *port_str = getenv("APP_PORT");
    const char *env_str  = getenv("APP_ENV");
    
    /*
     * 2. PROCESAR VARIABLE NUMÉRICA CON FALLBACK
     * Por defecto asumiremos el puerto 8080.
     */
    int port = 8080;
    if (port_str != NULL) {
        // atoi() convierte una cadena ASCII a un número entero (Ascii-TO-Integer).
        // Si la cadena contiene texto como "abc", atoi() devuelve 0.
        // En aplicaciones más robustas se prefiere strtol(), que sí 
        // informa si hubo un error en la conversión, pero aquí atoi() es suficiente.
        port = atoi(port_str);
    }
    
    /*
     * 3. PROCESAR VARIABLE DE TEXTO CON FALLBACK
     * Por defecto asumiremos el entorno "development".
     */
    const char *env = "development";
    if (env_str != NULL) {
        // Simplemente apuntamos nuestro puntero local a la dirección de memoria
        // de la variable de entorno descubierta.
        env = env_str;
    }
    
    /*
     * 4. APLICAR LA CONFIGURACIÓN (Simulado mediante impresión)
     */
    printf("Entorno actual: %s\n", env);
    printf("Escuchando en puerto: %d\n", port);
    
    return EXIT_SUCCESS;
}
