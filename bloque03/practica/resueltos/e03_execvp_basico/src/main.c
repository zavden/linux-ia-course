#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <cmd> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /*
         * argv + 1 apunta al comando real y sus argumentos.
         * execvp busca el binario en PATH.
         */
        execvp(argv[1], &argv[1]);

        /* Si llegamos aquí, exec falló */
        perror("execvp");
        _exit(127);
    }

    int st = 0;
    if (waitpid(pid, &st, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(st)) {
        printf("child_exit=%d\n", WEXITSTATUS(st));
    }

    return EXIT_SUCCESS;
}
