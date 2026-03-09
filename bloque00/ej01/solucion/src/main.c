/*
 * Ejercicio 0.1 — Hello Docker (SOLUCIÓN)
 * 
 * OBJETIVO DIDÁCTICO:
 * Este archivo muestra cómo interactuar con el sistema de archivos de Linux
 * desde C, específicamente leyendo un archivo de configuración estándar 
 * (/etc/os-release) para detectar información sobre la distribución actual.
 * Aplica conceptos de E/S básica, manejo de errores, y manipulación de strings.
 */

#include <stdio.h>   // Para fopen, fgets, printf, perror
#include <stdlib.h>  // Para EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>  // Para strncmp, strlen

// Es buena práctica definir rutas y constantes usando macros
// para que el código sea más legible y fácil de mantener.
#define OS_RELEASE_PATH "/etc/os-release"
#define PREFIX          "PRETTY_NAME="
#define LINE_MAX_LEN    256

int main(void) {
    /* 
     * 1. ABRIR EL ARCHIVO
     * fopen() abre un archivo. El modo "r" significa solo lectura (read).
     * Siempre debemos verificar si la apertura fue exitosa (puntero no nulo).
     */
    FILE *f = fopen(OS_RELEASE_PATH, "r");
    if (!f) {
        // perror() imprime nuestro mensaje personalizado seguido del error
        // del sistema (por ejemplo: "No such file or directory").
        perror("Error abriendo " OS_RELEASE_PATH);
        return EXIT_FAILURE; // Terminamos el programa con código de error (1).
    }

    /* 
     * 2. PREPARAR VARIABLES DE LECTURA
     * Reservamos un buffer en el stack (line[256]) para almacenar
     * cada línea leída del archivo temporalmente.
     */
    char line[LINE_MAX_LEN];
    int found = 0; // Bandera (flag) para saber si encontramos lo que buscábamos.

    /* 
     * 3. LEER LÍNEA POR LÍNEA
     * fgets() lee caracteres hasta encontrar un salto de línea ('\n') o 
     * llegar al límite (sizeof(line)). Retorna NULL cuando llega al final del archivo (EOF).
     */
    while (fgets(line, sizeof(line), f)) {
        
        /* 
         * 4. BUSCAR EL PREFIJO
         * strncmp compara los primeros N caracteres de dos cadenas.
         * Si retorna 0, significa que ambas cadenas son idénticas en esos N caracteres.
         */
        if (strncmp(line, PREFIX, strlen(PREFIX)) == 0) {
            
            /* 
             * 5. EXTRAER EL VALOR
             * 'line' es un puntero al inicio de la cadena. Al sumarle la longitud
             * del prefijo, hacemos que 'value' apunte justo al texto que nos interesa.
             * (Aritmética de punteros).
             */
            char *value = line + strlen(PREFIX);

            /* 
             * 6. LIMPIEZA DE LA CADENA (Sanitización)
             * Los archivos de configuración suelen terminar en '\n' y usar comillas.
             * Debemos remover ambos para imprimir una cadena limpia.
             */
            size_t len = strlen(value);
            
            // Si el último caracter es un salto de línea, lo reemplazamos por el terminador nulo ('\0')
            if (len > 0 && value[len - 1] == '\n') {
                value[--len] = '\0';
            }
            
            // Si la cadena está entre comillas dobles (ej: "Fedora Linux")
            // Reemplazamos la comilla final por '\0' y avanzamos el puntero 'value'
            // una posición para saltarnos la primera comilla.
            if (len > 1 && value[0] == '"' && value[len - 1] == '"') {
                value[len - 1] = '\0';
                value++;
            }

            /* 
             * 7. IMPRIMIR RESULTADO
             * Ahora 'value' apunta a una cadena limpia y terminada en nulo.
             */
            printf("Hello from %s\n", value);
            found = 1; // Marcamos éxito
            break;     // Salimos del loop, ya no necesitamos leer más el archivo.
        }
    }

    /* 
     * 8. CERRAR RECURSOS
     * Siempre debemos cerrar los archivos abiertos para liberar
     * el descriptor de archivo (file descriptor) que el sistema operativo nos asignó.
     */
    fclose(f);

    /* 
     * 9. MANEJO DE CASOS ATÍPICOS
     * ¿Qué pasa si el archivo existe pero no tiene PRETTY_NAME?
     */
    if (!found) {
        fprintf(stderr, "PRETTY_NAME no encontrado en %s\n", OS_RELEASE_PATH);
        return EXIT_FAILURE;
    }

    // Terminación exitosa (código 0 devuelto al sistema operativo / Docker)
    return EXIT_SUCCESS;
}
