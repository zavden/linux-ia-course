#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <comando> <args..> <archivo_salida>\n", argv[0]);
        // Ejemplo: ./app ls -la file.txt
        return EXIT_FAILURE;
    }

    // TODO: aislar dinámicamente el comando (ls), sus args ("-la") y el ultimísimo argumento que es target (file.txt).
    // TODO: Ejecutar fork().
    // TODO: El Hijo abre() el ultimo arg con O_WRONLY y mode.
    //       Ejecuta dup2 sobre STDOUT_FILENO
    //       Ejecuta execvp del comando. (Acuérdate de terminar la copia del vector de args de execvp con un NULL).
    //       Manejar si exec falla llamando perror(). exit(1).
    // TODO: El Padre espera wait()... Imprime éxito! 
    return EXIT_SUCCESS;
}
