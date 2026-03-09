#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Reto C01:
 * Shell con jobs.
 * Este archivo establece arquitectura y TODOs.
 */

/* TODO 1: estructura de jobs (pid, cmd, estado) */

static void sigchld_handler(int sig) {
    (void)sig;
    /*
     * TODO 2:
     * loop waitpid(-1, &st, WNOHANG)
     * marcar jobs terminados
     */
}

int main(void) {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    char line[1024];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        /*
         * TODO 3:
         * - parsear comando
         * - detectar '&'
         * - lanzar fork/exec
         * - si foreground: waitpid bloqueante
         * - si background: registrar job y continuar
         */

        if (strncmp(line, "exit", 4) == 0) {
            break;
        }
    }

    return EXIT_SUCCESS;
}
