#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void daemonize() {
    // TODO: fork 1 y exit
    // TODO: setsid()
    // TODO: fork 2 y exit
    // TODO: chdir('/') y umask(0)
    // TODO: redirigir fd 0,1,2 a /dev/null y dup2 a ellos
}

int main(void) {
    // TODO: Llenado signal_handlers (SIGTERM sale con exit, SIGHUP imprime local en .log recarga).
    // TODO: Crear pid file para no solapar dos en /tmp/mi_demonio.pid
    
    // daemonize();
    
    // while (flag_run) {
    //     log("Demonio Vivo Y Atendiendo...");
    //     sleep(2);
    // }
    
    // remove pid file
    return EXIT_SUCCESS;
}
