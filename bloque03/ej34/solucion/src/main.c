/*
 * Ejercicio 3.4 — Tiro al Blanco: Señales y sigaction (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Control Seguro de Eventos de Kernel asincrónicos. 
 * Se muestra el uso imperativo de variables `volatile sig_atomic_t` para evitar
 * Desastres de Hilos de Registro. También se usa `sigprocmask` para proteger al  
 * proceso (Ignorancia Temporal a Señales) durante su inicio frío.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

// `volatile`: Dile al compilador GCC (O2, O3) "Oye, yo sé que en el bucle
// parece que flag_run nunca cambia de estado, ¡Pero créeme que un evento fantasma  
// desde Afuera de tu scope de optimización podría pisarlo en este mismo instante!".
//
// `sig_atomic_t`: Te garantiza matemáticamente que en un OS de 64 bits, reescribir esta 
// variable jamás va a requerir la CPU frenar, dividir la instruccion, partirla en 2 bytes y
// sufrir si justo entre los 2 bytes entra el Handler. Escribirla será instantáneo, en 1 Clock Cycle ATÓMICo de tu CPU.
volatile sig_atomic_t flag_run = 1;
volatile sig_atomic_t sigint_count = 0;

/*
 * NOTA CRITICA DE INGENIERÍA:
 * En un manejador de señales, la gran amplia mayoría de funciones como `printf`, `malloc`, 
 * `free` son INSEGURAS (Non Async-Signal-Safe). Esto es porque si `malloc` estaba a 
 * medio manipular los punteros del Heap con Locks internos (`Mutex`) del Multihilo y lo interrumpes,
 * ¡Tendrás un Death Lock global del OS trágico e infinito imposible de rastrear!
 * La literatura nos manda estrictamente: Usa `write()` y variables globales y listo.
 */
void my_handler(int sig) {
    if (sig == SIGINT) {
        sigint_count++;
        if (sigint_count >= 3) {
            // El tercer Crtl+C al fin lo rompe. Mandamos señal de abandono al While Principal.
            flag_run = 0;
            const char *m = "\n    [HANDLER] Escudo SIGINT Destruido. Iniciando Caída Nuclear y Cleanup (3/3)...\n";
            write(STDOUT_FILENO, m, strlen(m));
        } else {
            // Un array para evitar un printf. Formateo cutre C.
            char msg[128];
            int len = snprintf(msg, sizeof(msg), "\n    [HANDLER] ¡JA! Sobrevivi al SIGINT/Ctrl+C (Golpes resistidos %d/3)\n", sigint_count);
            write(STDOUT_FILENO, msg, len);
        }
    } 
    else if (sig == SIGUSR1) {
        const char *m = "\t<Señal SIGUSR1 Privada Detectada> => La base de datos ha sido purgada (Simulacion!).\n";
        write(STDOUT_FILENO, m, strlen(m));
    }
    else if (sig == SIGTERM) {
        // Se considera que Terminate (15, kill nativo bash) siempre deba forzar shutdown sano.
        flag_run = 0;
        const char *m = "\n    [HANDLER] Muerte limpia dictada al Rey por SIGTERM (kill). Doblando rodilla pacifíca...\n";
        write(STDOUT_FILENO, m, strlen(m));
    }
}

int main(void) {
    printf("= Demonio C = Mi PID Activo Es: %d\n\n", getpid());

    /* ========================================================
     * INICIO PROTEGIDO / SECCIÓN CRÍTICA DE PROTECCIÓN (SIGPROCMASK)
     * Vamos a "Bloquear" Ctrl+C Temporalmente.
     * Si intentaran hacer Ctrl+C ahora, sus clicks no servirán, pero
     * Quedarán "Flotando" Mágicamente guardados en la cola de memoria Kernell... 
     * hasta que nosotros lo soltemos, golpeándonos 1 segundo después.
     * ======================================================== */
    sigset_t critical_mask;
    sigemptyset(&critical_mask);               // Deja la mascara blanca y virgen (vacía vacía 0000000)
    sigaddset(&critical_mask, SIGINT);         // Coloreamos SIGINT en la máscara

    // Activamos la máscara de ignorar de OS-Level
    sigprocmask(SIG_BLOCK, &critical_mask, NULL);
    
    printf("1) Arrancando Motores. Si aprietas Ctrl+C (SIGINT) AHORA mismo, el OS lo pausará hasta que yo termine esta carga...\n");
    sleep(3); // CARGA SIMULADA FALSA E IMPORTANTE

    // Desactivamos... Si habían pulsado Crtl+C, ¡Seremos golpeados bruscamente todos juntos Justo Ahora mismo!
    printf("2) Barreras bajadas. Puedes dar Crtl+C libremente para golpear mi escudo.\n");
    sigprocmask(SIG_UNBLOCK, &critical_mask, NULL);

    /* ========================================================
     * REGISTRO MODERNO DE MANEJADORES GLOBALES DE EXCEPCIONES POSIX
     * (El obsoleto `signal()` daba pesadillas a los seniors... ahora todo rige por sa)
     * ======================================================== */
    struct sigaction sa;
    sa.sa_handler = my_handler;
    sa.sa_flags = 0; // Opcional SA_RESTART: para autoconvenir Syscalls trabadas si quieres. Aca nos abstenemos.
    // Mientras la función HANDLER mía esté dentro del `if(SIGINT)` tratando de imprimir su
    // feo mensaje a Stderr, YO PIDO que tú temporalmente le BLOQUEES en el procesador la entrada (re-entrada infinita paralela fatal) 
    // al resto de mis de mis posibles signals! Se lo pido bloqueándole todos:
    sigfillset(&sa.sa_mask); 

    // Alquimia final. 
    if (sigaction(SIGINT, &sa, NULL) == -1 ||
        sigaction(SIGUSR1, &sa, NULL) == -1 ||
        sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Falla Fatal al invocar registro Kernel POSIX sigaction");
        exit(EXIT_FAILURE);
    }

    // ========================================================
    // MAQUINA ETERNA
    // ========================================================
    while (flag_run) {
        // En demonios reales no uses sleep ciego, haz select, o epoll o un pause() para despertar sólo en eventos puros sin consumir relojes CPU.
        pause(); // Suspende el thread a 0% CPU, y solo nos despierta "un balazo" de una Señal Registrada o NO-Registrada. 
    }

    printf("\n>>> Limpieza Exitosa General Al Salir del Ciclo (Cierre File Descriptors, BDs....) Apagado pacífico C++\n");

    return EXIT_SUCCESS;
}
