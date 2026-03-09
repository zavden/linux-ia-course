#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 64

/* Tokenización simple por espacios (didáctica, no shell completa) */
static int split_line(char *line, char **args, int max_args) {
    int n = 0;
    char *tok = strtok(line, " \t\r\n");
    while (tok && n < max_args - 1) {
        args[n++] = tok;
        tok = strtok(NULL, " \t\r\n");
    }
    args[n] = NULL;
    return n;
}

int main(void) {
    char line[1024];

    while (fgets(line, sizeof(line), stdin) != NULL) {
        char *args[MAX_ARGS];
        int argc = split_line(line, args, MAX_ARGS);

        if (argc == 0) {
            continue;
        }

        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return EXIT_FAILURE;
        }

        if (pid == 0) {
            execvp(args[0], args);
            perror("execvp");
            _exit(127);
        }

        int st = 0;
        if (waitpid(pid, &st, 0) == -1) {
            perror("waitpid");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
