#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    int down[2]; /* padre -> hijo */
    int up[2];   /* hijo -> padre */

    if (pipe(down) == -1 || pipe(up) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* Hijo: lee de down[0], escribe en up[1] */
        close(down[1]);
        close(up[0]);

        char in[128] = {0};
        ssize_t r = read(down[0], in, sizeof(in) - 1);
        if (r == -1) {
            perror("read hijo");
            _exit(1);
        }

        const char *prefix = "respuesta:";
        char out[160];
        snprintf(out, sizeof(out), "%s %s", prefix, in);

        if (write(up[1], out, strlen(out)) == -1) {
            perror("write hijo");
            _exit(1);
        }

        close(down[0]);
        close(up[1]);
        _exit(0);
    }

    /* Padre: escribe en down[1], lee de up[0] */
    close(down[0]);
    close(up[1]);

    const char *msg = "hola_hijo";
    if (write(down[1], msg, strlen(msg)) == -1) {
        perror("write padre");
        return EXIT_FAILURE;
    }
    close(down[1]);

    char resp[160] = {0};
    ssize_t rr = read(up[0], resp, sizeof(resp) - 1);
    if (rr == -1) {
        perror("read padre");
        return EXIT_FAILURE;
    }
    close(up[0]);

    printf("padre_recibio=%s\n", resp);

    int st = 0;
    waitpid(pid, &st, 0);
    return EXIT_SUCCESS;
}
