#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <out_file> <cmd> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *out_file = argv[1];

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        int fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("open");
            _exit(1);
        }

        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            close(fd);
            _exit(1);
        }

        close(fd);

        /* Comando empieza en argv[2] */
        execvp(argv[2], &argv[2]);
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
