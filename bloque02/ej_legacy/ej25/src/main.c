#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s <base> <hardlink> <symlink>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *origen = argv[1];
    const char *hlink = argv[2];
    const char *slink = argv[3];

    // TODO: Crear fichero y escribir
    // TODO: Syscalls link() y symlink()
    // TODO: stat() en base vs hlink para verificar st_ino (Inodes iguales)
    // TODO: lstat() en slink, readlink() para ver a donde apunta.
    // TODO: unlink(origen) (Puff, borrado falso, solo matamos el alias)
    // TODO: read(hlink) e imprimir para demostrar que los datos resistieron gracias al HardLink

    return EXIT_SUCCESS;
}
