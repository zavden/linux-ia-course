#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Reto C03:
 * Shell con redirecciones y pipeline mínimo.
 */

/* TODO 1: parser simple de tokens especiales | > >> < */
/* TODO 2: estructura de comando normalizada */
/* TODO 3: ejecución de comando simple con redirecciones */
/* TODO 4: ejecución de pipeline 2 etapas */

int main(void) {
    char line[1024];

    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (strncmp(line, "exit", 4) == 0) {
            break;
        }

        /*
         * TODO:
         * - detectar si hay pipe
         * - si no hay pipe: ejecutar comando simple con redirecciones
         * - si hay pipe: crear pipe, fork dos hijos, dup2 y execvp
         */

        printf("TODO: implementar parse/exec para: %s", line);
    }

    return EXIT_SUCCESS;
}
