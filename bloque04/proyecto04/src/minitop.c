#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>

void print_system_memory() {
    // TODO: Abrir /proc/meminfo con fopen()
    // Identificar strstr() líneas de Total, Libre, Cached y Buffers. Imprimir % real ocupado.
}

void print_system_load() {
    // TODO: Extraer desde /proc/loadavg y printear crudo.
}

int es_numerico(const char *str) {
    // TODO: loop de ctype isdigit(char). Retornar 0 si halla letra.
    return 1;
}

void parse_pid_folder(const char *pid_str) {
    // TODO: construir ruta /proc/%s/stat y /proc/%s/status
    // Leer primera y segunda columas nombre de (ejecutable)
    // Extraer RSS Memory (Opcional) desde status
    // Printear de forma "%-10s %-20s %s" la Tabla de procesos individual
}

int main(void) {
    // Bucle Refresh "while(1)"
        // printf("\033[2J\033[H"); // CLEAR ANSI Mágico
        // print_system_memory()
        // print_system_load()

        // DIR *proc = opendir("/proc");
        // readdir()
        // if(es_numerico(dirent->d_name)) parse_pid_folder()
        // closedir()

        // sleep(3)
        
    return EXIT_SUCCESS;
}
