/*
 * Ejercicio 3.5 — Magia Negra: Daemons SysV Style (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Implementar una demonización pura sin usar la función comodín anticuada 
 * `daemon()`. Enseña `setsid`, doble enmascaramiento y desvinculación terminal total. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

volatile sig_atomic_t g_run = 1;
volatile sig_atomic_t g_reload = 0;

/*
 * Escribiremos logs a un archivo porque ya no tendremos monitor (STDOUT=NULL)
 */
void write_log(const char *msg) {
    // Si queremos ser rudos, O_APPEND para que un Syslog no aplaste líneas.
    int fd = open("/tmp/mi_daemon_secreto.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd >= 0) {
        write(fd, msg, strlen(msg));
        close(fd);
    }
}

void daemon_handler(int sig) {
    if (sig == SIGTERM) {
        g_run = 0;
        write_log("[SIGNAL] SIGTERM Recibido. Terminando Demonio Correctamente.\n");
    } 
    else if (sig == SIGHUP) {
        g_reload = 1; 
        write_log("[SIGNAL] SIGHUP Recibido. (Trigger Para Recargar Configuracion!)\n");
    }
}

void daemonize(void) {
    // 1. PRIMER FORK (Rompemos dependencia del ancestro natural Shell)
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); // Padre original muere rápido y suelto para devolver prompt rápido al humano y terminal local. 

    // 2. DESVINCULACIÓN TOTAL DE SESIÓN Y TTY 
    if (setsid() < 0) exit(EXIT_FAILURE);

    // 3. SEGUNDO FORK (Magia de SysV) 
    // Ahora que somos los líderes de los grupos Ciegos de fondo que no ven pantallas, a nivel teórico
    // podríamos forzarnos a reconectar solicitando abrir un TTY local si somos audaces... a menos 
    // que nos forcemos a "NO SER EL LÍDER NUNCA MÁS" sacrificándonos mediante clonos.
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); // Líder del SID asesinado. Quedamos totalmente ciegos y mudos huérfanos del background.

    // 4. CHDIR y UMASK (Neutralizando bloqueos futuros y manipulaciones locales)
    umask(0);
    if (chdir("/") < 0) exit(EXIT_FAILURE); // Correrás bajo el System Root "/"

    // 5. CERRAR PUERTAS ESTANDARDS
    // stdin, stdout y stderr siguen conectadas fantasmagóricamente a la ventana terminal del humano C!
    // Las redirigimos al famoso basurero intergaléctico del SO (dev/null).
    int fd_null = open("/dev/null", O_RDWR);
    if (fd_null >= 0) {
        dup2(fd_null, STDIN_FILENO);
        dup2(fd_null, STDOUT_FILENO);
        dup2(fd_null, STDERR_FILENO);
        // Si fd_null fue un numero exótico superior a 2 (ej: 4, 30...) lo podemos descartar 
        // porque ya quedó clonado seguro y vivo sobre encima de las ID 0,1,2 de estándar C++ nativo.
        if(fd_null > 2) close(fd_null); 
    }
}

int main(void) {
    // TODO Aca puedes instanciar cualquier base de datos global.

    daemonize(); 
    // ==========================================
    // ¡EN ESTE PUNTO DE LÍNEA, LA TERMINAL ESTÁ LIBRE, Y TU CÓDIGO ESTA VIVO ATRÁS 
    // CORRIENDO EN EL ABISMO NOCTURNO DEL SERVER UBUNTU COMO DAEMON/SERVICIO!
    // ==========================================

    /* 
     * PID FILE LOCKING TÁCTICO
     * Asegura de que un administrador estúpido no ejecute dos instancias de mi NGINX 
     * creando colisiones por los mismos puertos. Exclusividad /tmp.
     */
    int pid_fd = open("/tmp/mi_daemon.pid", O_RDWR | O_CREAT | O_EXCL, 0644);
    if (pid_fd < 0) {
        // O_EXCL falló, lo que significa que el kernel advierte que el candado de .pid ¡Ya Fue Creado Antes!
        write_log("ERROR: Intento de instanciar un SEGUNDO daemon rebotado en seco. Lock adquirido.\n");
        exit(EXIT_FAILURE);
    }
    
    // Si fuimos exclusivos, reclamamos bandera.
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
    write(pid_fd, pid_str, strlen(pid_str));
    close(pid_fd);

    // BARRERA ANTIDISPARO KERNEL
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = daemon_handler;
    sigaction(SIGTERM, &sa, NULL); // Requerido para matar Daemones (kill normalito 15)
    sigaction(SIGHUP, &sa, NULL);  // Requerido para SysAdmin ConfigReloadings (kill -HUP 1)

    // CICLO VITAL 
    write_log("======== DAEMON INICIADO ========.\n");
    int i = 0;
    
    while (g_run) {
        if (g_reload) {
            write_log("==== APLICANDO CAMBIOS DE CONFIGURACION EN VIVO ====\n");
            g_reload = 0;
        }

        char buffer[128];
        snprintf(buffer, sizeof(buffer), "[Daemon] Tics de corazon vivos: %d segs...\n", i);
        write_log(buffer);
        
        sleep(2);
        i += 2;
    }

    // CIERRE Y DESTRUCCIÓN 
    unlink("/tmp/mi_daemon.pid");
    write_log("======== DAEMON DESTRUIDO LIMPIAMENTE ========\n");

    return EXIT_SUCCESS;
}
