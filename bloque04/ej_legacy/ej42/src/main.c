#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <archivo>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filepath = argv[1];

    // TODO: 1. Abrir open() archivo con O_RDWR.
    // TODO: 2. Buscar su tamanio total con fstat() o stat(). Guardalo para mmap.
    // TODO: 3. Ejecutar magico mmap(NULL, tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)
    // TODO: 4. Comprobar que no dio MAP_FAILED.
    // TODO: 5. Escribir/mutar el char 0. (Ej: data[0] = 'Z').
    // TODO: 6. Hacer flush magico: msync()
    // TODO: 7. Cerrar todo invocando munmap() y close().

    return EXIT_SUCCESS;
}
