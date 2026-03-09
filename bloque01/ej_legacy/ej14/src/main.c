#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// TODO: Crear tu macro mágica CHECK(call) aquí

int main(void) {
    FILE *f = NULL;
    char *buffer = NULL;
    int error_status = 1; // Asumimos fracaso por defecto

    // 1. Alojamos memoria dinámica
    buffer = malloc(1024);
    // TODO: Usar CHECK aquí

    // 2. Intentamos abrir un archivo que no existe
    f = fopen("archivo_fake.txt", "r");
    // TODO: Usar CHECK aquí (Cuidado fopen retorna NULL, no < 0, ajusta CHECK)

    printf("Si ves esto, el archivo existia y no saltó al cleanup.\n");
    error_status = 0; // Exito

// TODO: Crear la etiqueta 'cleanup:'
    // TODO: Si f no es nulo, fclose
    // TODO: Si buffer no es nulo, free
    
    return error_status ? EXIT_FAILURE : EXIT_SUCCESS;
}
