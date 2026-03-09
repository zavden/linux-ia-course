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

    char *end = NULL;
    long m = strtol(argv[2], &end, 8);
    if (*argv[2] == '\0' || *end != '\0' || m < 0 || m > 07777) {
        fprintf(stderr, "Modo inválido: %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    int fd = open(path, O_RDWR | O_CREAT, 0600);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    /*
     * fchmod sobre FD ya abierto evita algunas ventanas TOCTOU
     * frente a operar por path en dos pasos separados.
     */
    if (fchmod(fd, (mode_t)m) == -1) {
        perror("fchmod");
        close(fd);
        return EXIT_FAILURE;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);

    printf("mode=%03o\n", st.st_mode & 0777);
    return EXIT_SUCCESS;
}
