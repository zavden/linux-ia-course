#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Mapeo mínimo de errno a nombre estable para tests. */
static const char *errno_name(int err) {
    switch (err) {
        case ENOENT: return "ENOENT";
        case EACCES: return "EACCES";
        case EISDIR: return "EISDIR";
        default: return "OTHER";
    }
}

int main(void) {
    /* Ruta inexistente intencional para forzar ENOENT en cualquier máquina. */
    const char *path = "./archivo_que_no_existe.txt";

    /* fopen devolverá NULL y llenará errno con la causa del fallo. */
    FILE *f = fopen(path, "r");
    if (f) {
        fclose(f);
        fprintf(stderr, "el archivo no debería existir en este test\n");
        return EXIT_FAILURE;
    }

    /* Capturamos errno inmediatamente: siguientes llamadas podrían pisarlo. */
    int err = errno;
    /* Reporte con contexto: recurso + nombre de errno + mensaje del sistema. */
    printf("path=%s err_name=%s err_msg=%s\n", path, errno_name(err), strerror(err));

    return (err == ENOENT) ? EXIT_SUCCESS : EXIT_FAILURE;
}
