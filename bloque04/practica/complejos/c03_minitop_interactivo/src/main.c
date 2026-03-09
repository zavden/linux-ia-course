#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Reto C03:
 * monitor interactivo estilo top.
 */

struct proc_row {
    int pid;
    char name[64];
    long rss_kb;
    char state;
};

/* TODO 1: parser de meminfo/loadavg */
/* TODO 2: scan de /proc numérico y extracción por PID */
/* TODO 3: sort por rss desc */
/* TODO 4: clear screen + render tabla */

int main(void) {
    /*
     * TODO:
     * while (1)
     *   - recolectar métricas
     *   - ordenar
     *   - imprimir
     *   - sleep(intervalo)
     */
    puts("C03 plantilla lista");
    return EXIT_SUCCESS;
}
