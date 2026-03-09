#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static volatile sig_atomic_t got_usr1 = 0;

static void on_usr1(int sig) {
    (void)sig;
    got_usr1 = 1;
}

int main(void) {
    struct sigaction sa;
    sa.sa_handler = on_usr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    sigset_t block_set, old_set, pend_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGUSR1);

    /* Entramos en sección crítica bloqueando SIGUSR1 */
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) == -1) {
        perror("sigprocmask block");
        return EXIT_FAILURE;
    }

    raise(SIGUSR1);

    if (sigpending(&pend_set) == -1) {
        perror("sigpending");
        return EXIT_FAILURE;
    }

    printf("pending_usr1=%d\n", sigismember(&pend_set, SIGUSR1) == 1 ? 1 : 0);
    printf("got_usr1_while_blocked=%d\n", (int)got_usr1);

    /* Salimos de sección crítica y permitimos entrega */
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) == -1) {
        perror("sigprocmask restore");
        return EXIT_FAILURE;
    }

    usleep(100000);
    printf("got_usr1_after_unblock=%d\n", (int)got_usr1);

    return EXIT_SUCCESS;
}
