#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    int p[2];
    if (pipe(p) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(p[0]);
        close(p[1]);
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* Hijo: solo lee */
        close(p[1]);

        char buf[128] = {0};
        ssize_t r = read(p[0], buf, sizeof(buf) - 1);
        close(p[0]);

        if (r == -1) {
            perror("read");
            _exit(1);
        }

        for (ssize_t i = 0; i < r; ++i) {
            buf[i] = (char)toupper((unsigned char)buf[i]);
        }

        printf("hijo_recibio=%s\n", buf);
        fflush(stdout);
        _exit(0);
    }

    /* Padre: solo escribe */
    close(p[0]);

    const char *msg = "hola pipe";
    if (write(p[1], msg, strlen(msg)) == -1) {
        perror("write");
        close(p[1]);
        return EXIT_FAILURE;
    }

    close(p[1]);

    int st = 0;
    waitpid(pid, &st, 0);
    return EXIT_SUCCESS;
}
