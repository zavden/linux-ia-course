/*
 * Ejercicio 3.2 — Posesión: exec, fork y dup2 (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Replicar el alma del shell. Cómo `bash` interpreta la flecha (>) de terminal
 * para atrapar outputs visuales de comandos externos ajenos.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // fork, execvp, dup2, STDOUT_FILENO
#include <sys/wait.h> // waitpid
#include <fcntl.h>    // open, O_WRONLY...
#include <string.h>

int main(int argc, char *argv[]) {
    // Requerimos al menos: ./app <programa> <archivo_destino>
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <comando> [args_opcionales...] <archivo_salida>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* 
     * MANIPULACIÓN DE ARGUMENTOS
     * execvp exige imperativamente un array de strings que TEMINE CON NULL.
     * argv ya de por sí termina así (argv[argc] == NULL garantizado).
     *
     * Queremos aislar el ÚLTIMO argumento del usuario para que sea el Archivo,
     * pero NO queremos enviárselo al comando intruso si no se colgará pensando
     * que es un parámetro de entrada para él.
     */
    char *output_file = argv[argc - 1];

    // Clavamos una traba de string NULL justo donde estaba el archivo, mutilando
    // artificialmente la lista de argv del sistema y engañando a execvp para que
    // piense que los argumentos acaban un índice antes. 
    // Magia pura de C con arrays.
    argv[argc - 1] = NULL; 

    // argv[1] es nuestro comando a ejecutar ahora ("ls", "cat", etc.)
    char *cmd = argv[1];

    // ============================================

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork aborotado, Linux rechazo al hijo");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) {
        // ===================================
        // CONTEXTO [HIJO]
        // Su meta es asfixiarse en execvp para reencarnar cargando el Binario
        // ===================================
        
        // 1. Abrir la prisión donde se alojarán los bytes
        int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out < 0) {
            perror("El Archivo Destino no es escribible");
            exit(EXIT_FAILURE);
        }

        // 2. MAGIA DE DESVÍO STDOUT - DUP2
        // Desenchufo el Identificador universal STDOUT_FILENO (Que valia 1 = Pantalla)
        // Y LO COLOCO ENCIMA DEL Identificador de mi archivo físico local.
        if (dup2(fd_out, STDOUT_FILENO) < 0) {
            perror("Error de plomeria (dup2 STDOUT)");
            exit(EXIT_FAILURE);
        }

        // Ya asegurada la conexion, el socket original no se necesita por duplicado.
        close(fd_out);

        // 3. POSESIÓN DIABÓLICA (EXECVP)
        // Desde este punto exacto, el código de "app.c" cesará de existir para este hijo.
        // Toda la memoria ram sera barrida, y se subirá al OS el código Binario Ensamblado de "cmd".
        // NOTA: Como la pantalla Stderr (Id error: 2) NO LA MANIPULAMOS, si el "ls" crashea o da fatalidades,
        // lo imprimirá limpio en nuestra terminal base en vez de en el archivo del error crudo.
        execvp(cmd, &argv[1]); 

        // ====== LA LÍNEA DE LOS CAÍDOS ======
        // JAMÁS se alcanzará esta línea SI la posesión binaria funcionó.
        // Si el CPU la lee... significa miserablemente que "cmd" no existía en el $PATH
        // y execvp retornó una humillante queja arrojando -1 a C.
        perror("\t[HIJO] => Falla la transformacion de execvp. Comando perdido");
        exit(EXIT_FAILURE);
    } 
    else {
        // ===================================
        // CONTEXTO [PADRE]
        // ===================================
        int wstatus;
        waitpid(pid, &wstatus, 0);

        if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus) == 0) {
            printf("[PADRE] => Orquestación exitosa. Output canalizado silenciósamente en archivo: '%s'\n", output_file);
        } else {
            printf("[PADRE] => Alerta, la orquestación derivó fallas (Inyección abortada o el programa del usuario botó salida no-cero).\n");
            // Puedes comprobar errores revisando WEXITSTATUS si quisieses.
        }
    }

    return EXIT_SUCCESS;
}
