#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // 1. Validar args
    if (argc != 2) {
        printf("Uso: %s <directorio>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *dir_path = argv[1];

    // TODO: Abrir el directorio con opendir().
    // TODO: Iterar sobre entradas con readdir().
    // TODO: Para cada entrada, crear el path completo con sprintf/snprintf.
    // TODO: Llamar a stat() o lstat() para sacar metadatos.
    // TODO: Imprimir tamanio y nombre o tipo.
    // TODO: Cerrar el directorio con closedir()

    return EXIT_SUCCESS;
}
