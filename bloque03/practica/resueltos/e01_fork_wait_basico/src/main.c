#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    const int n = 3;

    /*
     * Creamos N hijos. Cada hijo sale con código distinto.
     * Esto permite validar que waitpid está recogiendo correctamente.
     */
    for (int i = 0; i < n; ++i) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            return EXIT_FAILURE;
        }

        if (pid == 0) {
            /* Código del hijo */
            printf("hijo pid=%d ppid=%d idx=%d\n", (int)getpid(), (int)getppid(), i);
            _exit(10 + i);
        }
    }

    /* Padre: recolecta exactamente N hijos */
    for (int k = 0; k < n; ++k) {
        int st = 0;
        pid_t w = waitpid(-1, &st, 0);
        if (w == -1) {
            perror("waitpid");
            return EXIT_FAILURE;
        }

        if (WIFEXITED(st)) {
            printf("recolectado pid=%d code=%d\n", (int)w, WEXITSTATUS(st));
        }
    }

    return EXIT_SUCCESS;
}
