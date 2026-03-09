#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Reto C02:
 * Daemon robusto con lifecycle completo.
 */

static volatile sig_atomic_t running = 1;
static volatile sig_atomic_t reload_cfg = 0;

static void on_sig(int sig) {
    if (sig == SIGTERM) running = 0;
    if (sig == SIGHUP) reload_cfg = 1;
}

/* TODO 1: daemonize doble fork */
static int daemonize(void) {
    return 0;
}

/* TODO 2: lock pidfile y validación de instancia única */
/* TODO 3: bucle principal de logging + reload */

int main(void) {
    struct sigaction sa;
    sa.sa_handler = on_sig;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);

    if (daemonize() == -1) {
        return EXIT_FAILURE;
    }

    while (running) {
        if (reload_cfg) {
            reload_cfg = 0;
            /* TODO: recargar config */
        }
        sleep(1);
    }

    return EXIT_SUCCESS;
}
