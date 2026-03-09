#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv) {
    /* El ejercicio opera sobre un archivo real pasado por argumento. */
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Abrimos en lectura/escritura porque mutaremos contenido mapeado. */
    int fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }

    /*
     * mmap de longitud 0 no tiene sentido práctico en este contexto:
     * pedimos al alumno trabajar con un archivo no vacío.
     */
    if (st.st_size == 0) {
        fprintf(stderr, "archivo vacío no mapeable\n");
        close(fd);
        return EXIT_FAILURE;
    }

    /*
     * Mapping compartido: cambios pueden persistir al archivo.
     */
    char *data = mmap(NULL, (size_t)st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    /* Escribimos el primer byte directamente en memoria mapeada. */
    data[0] = 'X';

    /* Fuerza de persistencia al archivo subyacente antes de desmontar. */
    if (msync(data, (size_t)st.st_size, MS_SYNC) == -1) {
        perror("msync");
        munmap(data, (size_t)st.st_size);
        close(fd);
        return EXIT_FAILURE;
    }

    /* Limpieza: primero munmap, luego close del file descriptor. */
    munmap(data, (size_t)st.st_size);
    close(fd);

    printf("ok_size=%lld\n", (long long)st.st_size);
    return EXIT_SUCCESS;
}
