/*
 * Ejercicio 0.4 — Valgrind y GDB (SOLUCIÓN DIDÁCTICA)
 * 
 * OBJETIVO DIDÁCTICO:
 * Demostrar qué es un "Memory Leak" (fuga de memoria) y un "Segmentation Fault"
 * (violación de segmento), y cómo solucionarlos. El objetivo es que Valgrind y GDB
 * ya no se quejen de estos errores tras aplicar los fixes (que aquí ya están hechos).
 */

#include <stdio.h>  // perror, printf
#include <stdlib.h> // malloc, free, exit, EXIT_SUCCESS, EXIT_FAILURE
#include <string.h> // strcpy

void process_data(void) {
    /* 
     * 1. ALOCACIÓN DINÁMICA DE MEMORIA (HEAPS)
     * malloc(tamaño_en_bytes) pide memoria al sistema operativo.
     * Esta memoria queda asignada ("bloqueada") hasta que tú explícitamente la devuelvas.
     */
    char *buffer = malloc(1024);
    
    // Siempre, SIEMPRE verifica si malloc devuelve NULL. (Podría no haber RAM).
    if (!buffer) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    // Escribimos información en nuestro buffer dinámico.
    strcpy(buffer, "Procesando informacion confidencial...");
    printf("Mensaje: %s\n", buffer);

    /* 
     * 2. LIBERACIÓN DE MEMORIA (FIX DEL LEAK)
     * EN EL PROBLEMA ORIGINAL: La función terminaba aquí y el bloque de 1024 bytes
     * se quedaba huérfano. El puntero 'buffer' se destruía al salir de la función,
     * pero la RAM seguía bloqueada en el OS -> Memory Leak ("definitely lost" en Valgrind).
     * 
     * SOLUCIÓN: Usamos free() pasándole el puntero a la memoria que ya no necesitamos.
     */
    free(buffer);
}

void trigger_crash(void) {
    /*
     * 3. VIOLACIÓN DE ACCESO A MEMORIA (SEGFAULT)
     * EN EL PROBLEMA ORIGINAL: Había el siguiente código comentado:
     *   int *ptr = NULL; 
     *   *ptr = 42;
     * 
     * Tratar de leer o escribir en la dirección reservada 0x0 (NULL) está 
     * terminantemente prohibido por el OS para aislar los procesos.
     * Esto causa una señal SIGSEGV inmediata que cierra el programa.
     * En GDB, usaríamos 'bt' (backtrace) para ver qué línea mató al programa.
     * 
     * SOLUCIÓN: Hacemos que el puntero apunte a una dirección legal y válida 
     * (la dirección de la variable valid_value en el stack).
     */
    int valid_value = 42;
    int *ptr = &valid_value; // Ahora ptr apunta a un bloque de memoria legal de la pila.
    *ptr = 42;               // Ahora esto es totalmente seguro y legal.
}

int main(void) {
    printf("Iniciando aplicación...\n");
    
    process_data();
    trigger_crash();
    
    // Si Valgrind corre este código solucionado, dirá felizmente:
    // "All heap blocks were freed -- no leaks are possible"
    
    printf("Aplicación finalizada correctamente.\n");
    return EXIT_SUCCESS;
}
