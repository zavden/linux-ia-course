#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void *my_memcpy(void *dest, const void *src, size_t n) {
    // TODO: Castear dest y src a (char*).
    // TODO: Parchar 'n' bytes manualmente iterando con un for (dest[i] = src[i];)
    return dest;
}

int main(void) {
    size_t buffer_size = 50 * 1024 * 1024; // 50 MB
    // TODO: Alojar src y dest buffers
    
    // TODO: Tomar tiempo antes
    // my_memcpy(...)
    // Tomar tiempo despues y calcular diff
    
    // Repetir validando con memcpy nativo de C
    
    // Imprimir comparación en Segundos / Milisegundos
    return EXIT_SUCCESS;
}
