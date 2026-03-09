#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Macro robusta para llamadas tipo syscall:
 * - evalúa la expresión una sola vez
 * - imprime contexto útil
 * - salta a cleanup
 */
#define CHECK_SYS(call)                                                       \
    do {                                                                      \
        if ((call) == -1) {                                                   \
            fprintf(stderr, "[%s:%d] %s falló: %s\n",                       \
                    __FILE__, __LINE__, #call, strerror(errno));              \
            goto cleanup;                                                     \
        }                                                                     \
    } while (0)

int main(void) {
    int fd = -1;
    int rc = EXIT_FAILURE;

    /* Fallará intencionalmente para demostrar el flujo */
    CHECK_SYS(fd = open("/archivo/falso.txt", O_RDONLY));

    rc = EXIT_SUCCESS;

cleanup:
    if (fd != -1) {
        close(fd);
    }
    return rc;
}
