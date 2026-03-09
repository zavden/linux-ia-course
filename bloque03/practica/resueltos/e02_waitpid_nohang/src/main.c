#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        sleep(1);
        _exit(42);
    }

    /*
     * Padre: consulta no bloqueante hasta que el hijo termine.
     * Ideal para loops de evento/shells que no deben congelarse.
     */
    for (;;) {
        int st = 0;
        pid_t r = waitpid(pid, &st, WNOHANG);
        if (r == -1) {
            perror("waitpid");
            return EXIT_FAILURE;
        }

        if (r == 0) {
            printf("esperando...\n");
            usleep(200000);
            continue;
        }

        if (WIFEXITED(st)) {
            printf("recolectado code=%d\n", WEXITSTATUS(st));
        }
        break;
    }

    return EXIT_SUCCESS;
}
