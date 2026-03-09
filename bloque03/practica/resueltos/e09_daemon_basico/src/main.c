#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int daemonize(void) {
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid > 0) _exit(0);

    if (setsid() == -1) return -1;

    pid = fork();
    if (pid < 0) return -1;
    if (pid > 0) _exit(0);

    umask(0);
    if (chdir("/") == -1) return -1;

    int nullfd = open("/dev/null", O_RDWR);
    if (nullfd == -1) return -1;

    dup2(nullfd, STDIN_FILENO);
    dup2(nullfd, STDOUT_FILENO);
    dup2(nullfd, STDERR_FILENO);

    if (nullfd > 2) close(nullfd);
    return 0;
}

int main(void) {
    if (daemonize() == -1) {
        return EXIT_FAILURE;
    }

    FILE *f = fopen("/tmp/e09_daemon.log", "a");
    if (!f) {
        return EXIT_FAILURE;
    }

    for (int i = 1; i <= 3; ++i) {
        fprintf(f, "tick=%d\n", i);
        fflush(f);
        sleep(1);
    }

    fclose(f);
    return EXIT_SUCCESS;
}
