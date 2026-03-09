#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void process_data(void) {
    // Alocar 1KB de memoria dinamicamente
    char *buffer = malloc(1024);
    if (!buffer) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    strcpy(buffer, "Procesando informacion confidencial...");
    printf("Mensaje: %s\n", buffer);

    // ERROR INTENCIONAL 1: Falta liberar la memoria.
    // Añade el free() necesario aquí abajo.
}

void trigger_crash(void) {
    // ERROR INTENCIONAL 2: Segmentation fault
    // Descomentar la siguiente línea para depurar con GDB:
    // int *ptr = NULL; *ptr = 42; 
}

int main(void) {
    printf("Iniciando aplicación...\n");
    
    process_data();
    trigger_crash();
    
    printf("Aplicación finalizada correctamente.\n");
    return EXIT_SUCCESS;
}
