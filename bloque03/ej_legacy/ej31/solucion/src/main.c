/*
 * Ejercicio 3.1 — fork() y waitpid() (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Introducir la paralelización nativa del Kernel Linux mediante copias pesadas.
 * Comprender el aislamiento de procesos y cómo una macro como WEXITSTATUS 
 * desempaca la información oculta rescatada del cuerpo (entero del socket waitpid).
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // fork(), getpid(), getppid(), sleep()
#include <sys/wait.h> // waitpid() y macros W*
#include <errno.h>

// Esta variable existe en el segmento DATA en RAM antes del fork().
// Tras el fork, cada hijo gana su "copia pirata" personal, y pueden destrozarla
// sin que se enteren mutuamente los demás ni el proceso Padre originario.
int isolated_var = 1000;

int main(void) {
    int max_hijos = 3;

    printf("[Padre] Mi PID Principal Inmortal es: %d\n", getpid());

    // 1. LA CLONACIÓN
    for (int i = 0; i < max_hijos; i++) {
        pid_t p = fork();

        if (p < 0) {
            // El OS colapsó asfixiado de forkbombs o no hay ram base
            perror("Error bifurcando (fork falló)");
            exit(EXIT_FAILURE); 
        }
        else if (p == 0) {
            // ================== ESPACIO DE LOS  HIJOS ==================
            // Si bien el código está compartido con el mismo string del padre
            // Ellos lográn saltar adentro del clausulado If mágico y los demás no.
            isolated_var += 500; // Intento ingenuo de corromper la memoria Base
            
            printf("  [Hijo %d] He nacido. Mi PID: %d | PID de mi Papi: %d\n", 
                   i, getpid(), getppid());
                   
            sleep(1); 
            
            // Aborta y sepulta RAM del hijo. Envía el byte final cifrado (-255)
            exit(10 + i); 
            // CUALQUIER CODIGO MAS ABAJO ACA NUNCA EN CIENTOS DE MILENIOS EJECTUTARÍA..
            // ========================================================
        } 
        // El ELSE implícito si "p > 0" es solo el Padre, porque está iterando limpiamente 
        // el for despachándolos al OS mientras él sigue en sus cosas
    }

    // 2. EL VELATORIO Y RECOLECCION
    // El Padre se dedicó a correr asíncronamente mientras los 3 dormían
    printf("[Padre] Esperando el velorio de los %d difuntos...\n", max_hijos);

    for (int i = 0; i < max_hijos; i++) {
        int wstatus;
        
        // pid = -1 en waitpid significa "Espera a la muerte del PRIMER hijo cualquiera 
        // en cola, idéntico a usar una llama a primitiva a wait()"
        pid_t victim_pid = waitpid(-1, &wstatus, 0);
        
        if (victim_pid == -1) {
            perror("Waitpid asimetrico roto o inexistente");
            continue;
        }

        // Descifrando la causa del fallecimiento y la metadata salvada de los restos
        if (WIFEXITED(wstatus)) {
            // Falleció con causa natural: un exit(XXX) ejecutado dentro del código
            int exit_code = WEXITSTATUS(wstatus);
            printf("[Padre] Hijo %d exhaló en reposo y escupió Código: %d\n", victim_pid, exit_code);
        } 
        else if (WIFSIGNALED(wstatus)) {
            // Falleció asesinado por balas software (Ej. usuario le disparó `kill -9`)
            int sig = WTERMSIG(wstatus);
            printf("[Padre] Hijo %d asesinado brutalmente por la Señal Kernell Nº %d\n", victim_pid, sig);
        }
    }

    // 3. AISLAMIENTO VERIFICADO
    printf("[Padre] Todos fueron sepultados (No hay Zombies). ¿Pudieron hackearte tu Variable Local Mítica?\n");
    printf("[Padre] isolated_var = %d. %s\n", isolated_var, 
           (isolated_var == 1000) ? "(Intacta! Aislamiento de Linux Garantizado!)" : "Te hackearon la matrix!!");

    return EXIT_SUCCESS;
}
