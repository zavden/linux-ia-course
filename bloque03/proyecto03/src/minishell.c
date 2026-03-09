#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_ARGS 64

void sigchld_handler(int sig) {
    // TODO: Usar waitpid(-1, NULL, WNOHANG) en un bucle while para reapear zombies
}

int main(void) {
    // TODO: Ignorar SIGINT en padre
    // TODO: Manejar SIGCHLD

    char input[1024];
    while (1) {
        // 1. Mostrar Prompt con ruta getcwd()
        // 2. Leer input y parsear. Fijarse en pipes '|' y tokens de args '&'
        // 3. Ejecutar Builtins (cd, exit, pwd)
        // 4. Hacer forks con manejo de execvp... 
        // 5. Soporte simple para tuberías si hay en el texto partido.
    }

    return EXIT_SUCCESS;
}
