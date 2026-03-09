#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {
    // 1. Validar args
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <origen> <destino>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *origen = argv[1];
    const char *destino = argv[2];

    int fd_in = -1;
    int fd_out = -1;

    // TODO: Abrir fd_in con open() en modo O_RDONLY
    // TODO: Abrir fd_out con open() en modo O_WRONLY | O_CREAT | O_TRUNC y permisos 0644
    // TODO: Leer de fd_in en un buffer y escribir en fd_out hasta que read devuelva 0
    // TODO: Usar `close()` para limpieza en ambos FDs. No olvides chequear errores.

    return EXIT_SUCCESS;
}
