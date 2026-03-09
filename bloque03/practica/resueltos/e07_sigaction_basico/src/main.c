#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Variables globales asíncronas:
 * - sig_atomic_t evita lecturas parciales en contexto de señal.
 * - volatile evita optimizaciones peligrosas para polling.
 */
static volatile sig_atomic_t usr1_count = 0;
static volatile sig_atomic_t stop_flag = 0;

static void handler(int sig) {
    if (sig == SIGUSR1) {
        usr1_count++;
    } else if (sig == SIGTERM) {
        stop_flag = 1;
    }
}

int main(void) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGUSR1, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    printf("pid=%d\n", (int)getpid());

    sig_atomic_t last = -1;
    while (!stop_flag) {
        if (usr1_count != last) {
            last = usr1_count;
            printf("usr1_count=%d\n", (int)usr1_count);
            fflush(stdout);
        }
        usleep(100000);
    }

    printf("stop=1\n");
    return EXIT_SUCCESS;
}
