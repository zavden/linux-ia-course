#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <archivo> <modo_octal>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *path = argv[1];

    /*
     * strtol base 8 para convertir "0644" -> valor octal real.
     * Validación estricta de parseo para evitar entradas ambiguas.
     */
    char *end = NULL;
    long mode_long = strtol(argv[2], &end, 8);
    if (*argv[2] == '\0' || *end != '\0' || mode_long < 0 || mode_long > 07777) {
        fprintf(stderr, "Modo inválido: %s\n", argv[2]);
        return EXIT_FAILURE;
    }
    mode_t mode = (mode_t)mode_long;

    /* Guardamos y neutralizamos umask temporalmente */
    mode_t old = umask(0);

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);

    /* Restauramos umask incluso si open falla */
    umask(old);

    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
    close(fd);

    struct stat st;
    if (stat(path, &st) == -1) {
        perror("stat");
        return EXIT_FAILURE;
    }

    printf("mode=%03o\n", st.st_mode & 0777);
    return EXIT_SUCCESS;
}
