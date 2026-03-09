#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int global_var = 100;

int main(void) {
    int num_hijos = 3;

    for (int i = 0; i < num_hijos; i++) {
        // TODO: Llamar a fork()
        // TODO: Si es 0 (Hijo):
        //       - Sumarle a global_var algo
        //       - Hacer printf con getpid() y getppid()
        //       - sleep(1);
        //       - exit(i + 10);
    }

    // TODO: Si es Padre (ya salio del for de creacion):
    //       Hacer un solo blucle recolectanto "num_hijos" times:
    //       Usar waitpid()
    //       Imprimir WIFEXITED y WEXITSTATUS
    
    // Imprimir global_var final para demostrar que no cambio en el padre
    
    return EXIT_SUCCESS;
}
