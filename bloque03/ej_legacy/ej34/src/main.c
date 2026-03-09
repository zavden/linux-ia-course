#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// TODO: Definir volatile sig_atomic_t flag_run = 1;
// TODO: Definir volatile sig_atomic_t sigint_count = 0;

void my_handler(int sig) {
    // TODO: Switch(sig). Si SIGINT -> count++. Si count==3 -> flag_run=0
    //       Si SIGUSR1 -> Imprimir mensaje local
    //       Si SIGTERM -> Imprimir algo y flag_run=0
    // Recordatorio: printf es no asincrono y mala practica en señales (pero lo usaremos por demo y aprendizaje). Aca se suele usar solo write().
}

int main(void) {
    // TODO: llenar una struct sigaction. 
    // TODO: sigemptyset(&sa.sa_mask), sa.sa_handler = my_handler;
    // TODO: sigaction(SIGINT, ...), sigaction(SIGUSR1, ...), sigaction(SIGTERM, ...)

    // TODO: Demostracion Sección critica (Bloqueo temporal)
    // Crear una mask de bloqueo: sigemptyset -> sigaddset(SIGINT) -> sigprocmask(SIG_BLOCK)
    // Dormir sleep(3) 
    // sigprocmask(SIG_UNBLOCK)

    printf("Inicializacion completada. Lanzame señales con kill -USR1 %d o apretando Ctrl+C 3 veces.\n", getpid());
    
    // while(flag_run) { sleep(1); }

    printf("Limpieza Exitosa y Apagado pacifico del servicio alcanzado por fin!\n");

    return EXIT_SUCCESS;
}
