/*
 * Ejercicio 1.2 — main.c para probar safe_strings
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "safe_strings.h"

int main(void) {
    char small_buffer[10];

    // Prueba 1: safe_strcpy con sobrecarga masiva
    // Un strcpy común hubiera provocado stack smashing aquí.
    const char *danger = "String maliciosa supremamente enorme que reventaria un buffer de diez caracteres si se deja suelta";
    
    size_t required = safe_strcpy(small_buffer, danger, sizeof(small_buffer));
    
    printf("--- Prueba 1: Copia ---\n");
    printf("Buffer Resultante: '%s'\n", small_buffer);
    printf("Largo del texto: %zu, Requerido (real): %zu\n", strlen(small_buffer), required);
    if (required >= sizeof(small_buffer)) {
        printf("[!] Aviso de truncamiento detectado correctamente en la capa segura.\n\n");
    }

    // Prueba 2: safe_strcat
    char dest_cat[15];
    safe_strcpy(dest_cat, "Hola ", sizeof(dest_cat));
    safe_strcat(dest_cat, "Mundo cruel e injusto", sizeof(dest_cat)); // Se desborda
    
    printf("--- Prueba 2: Concat ---\n");
    printf("Buffer Resultante: '%s'\n", dest_cat);
    
    // Prueba 3: safe_snprintf
    char print_buf[20];
    safe_snprintf(print_buf, sizeof(print_buf), "Hola %s, el numero es %d y mas texto...", "Alejandro", 123456789);
    
    printf("--- Prueba 3: Snprintf ---\n");
    printf("Buffer Resultante: '%s'\n", print_buf);

    return EXIT_SUCCESS;
}
