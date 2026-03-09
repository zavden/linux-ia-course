#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(void) {
    // 1. Array de dos enteros para cada tubo (Pipe)
    // indice 0 = LECTURA, indice 1 = ESCRITURA
    int pipe_down[2]; // Direccion: Padre -> Hijo
    int pipe_up[2];   // Direccion: Hijo -> Padre

    // TODO: Usar syscall pipe(pipe_down) y control de errores...
    // TODO: Syscall pipe(pipe_up)...

    // TODO: fork()
    // Si es HIJO (0):
    //    1. Cerrar extremos inutiles: (pipe_down[1] y pipe_up[0])
    //    2. Leer string original desde pipe_down[0]
    //    3. Responder un mensaje por pipe_up[1]
    //    4. Cerrar lo ultimo y exit(0)
    
    // Si es PADRE:
    //    1. Cerrar extremos inutiles: (pipe_down[0] y pipe_up[1])
    //    2. Enviar string "Mision X" por pipe_down[1]
    //    3. Leer buffer respuesta desde pipe_up[0] y hacer un printf final
    //    4. wait()
    
    return EXIT_SUCCESS;
}
