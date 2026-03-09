/*
 * Ejercicio 6.2 — I/O No Bloqueantes C Native (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Controlar y moldear un Read Kernel hacia EAGAIN asíncrono para
 * evadir trabas (Polling manual primitivo base).
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


int main(void) {
    printf("============== EVENT LOOP POLLING PRIMITIVO (El Core_0 de Node) ==============\n\n");

    /* MAGIA C OS POSIX
     * Cambiar el comportamiento de Fábrica de Linux Bloqueante 
     * Hacia un Sistema de Respuestas Rápidas (O_NONBLOCK).
     */
    int banderas_viejas = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (banderas_viejas == -1) {
         perror("Fatal Linux FD Mapeo Get"); return EXIT_FAILURE;
    }

    // Le inyectamos (OR de Bits puro ++) a las previas flags nuestra brujería nonblock
    if (fcntl(STDIN_FILENO, F_SETFL, banderas_viejas | O_NONBLOCK) == -1) {
         perror("Fatal Linux FD Mapeo Set Async"); return EXIT_FAILURE;
    }
    
    printf("\033[1;33m[!] Teclado (FD 0) pasado Atómicamente Kernell Mode A: NON-BLOCKING\033[0m\n");
    printf("[!] Intenta tipear palabras o escribe 'quit' para Matar Event Loop Asignado.\n\n");

    // ===================================
    // EL EVENT LOOP MAGNO ASIYNCRONO DE 1 CLUSTER
    // ===================================
    while (1) {
        char in_buffer[128];
        
        // 1. ATAQUE DIRECTO
        // Si no fueramos Non-Block... El programa moriria trabado un año entero en esta linea inferior.
        ssize_t b_leidos = read(STDIN_FILENO, in_buffer, sizeof(in_buffer) - 1);

        // 2. RAMA DE RESPUESTA RAPIDA (SI SEÑALAMOS ENTER)
        if (b_leidos > 0) {
             in_buffer[b_leidos] = '\0';
             // Borramos el fastidioso NewLine para log limpio
             if (in_buffer[b_leidos - 1] == '\n') in_buffer[b_leidos - 1] = '\0';
             
             printf("\n   \033[1;32m[+] EVENT_TRIVIA C_OS: Capturaste un Teclado Asíncrono puro: '%s'\033[0m\n", in_buffer);

             if (strcmp(in_buffer, "quit") == 0) {
                 printf("    => Comando C_OS de salida Acatado. Abortando Polling Magno...\n");
                 break;
             }
        } 
        // 3. LA GRAN MAGIA EVASIVA OS KERNEL (EAGAIN)
        else if (b_leidos < 0) {
             // EAGAIN significa "Try Again Later" y EWOULDBLOCK "Se Hubiera Bloqueado pero te salvé master!". 
             // En algunas Arq Posix y compildodores Glibc son el mismo MACRO NÚMERO e igual sirven!.
             if (errno == EAGAIN || errno == EWOULDBLOCK) {
                  // Todo Bien, Simplemente... Aún el men no pulsa.
                  printf(".");
                  fflush(stdout); // Fuerzo vaciado visual del puntito pq Printf suele Cachear si no hay \n
             } else {
                  perror("\nFallo misterioso C_Error (Tu teclado USB fue arrancado o quemado de PC Kernell?)");
                  break; 
             }
        }
        else {
             // LEyo Exactamente 0 Bytes. Esto en C Unix significa -> "Fin de Archivo C_EOF". (Hicieron Ctrl+D en Bash Unix!)
             printf("\n   [!] Evento EOF. Me apagaron la terminal C_OS Crtl+D Asíncronda!! Abortando.\n");
             break;
        }


        /* 
         * MUY IMPORTANTE EL SLEEP(POLLING C)
         * Si dejas al EventLoop C Puro correr un PENTIUM A 4.0Ghz ... hará MILLONES DE VUELTAS por segundo 
         * y llamar a `read` 4 Millones de veces por seg quemará tu disco duro y derretirá tu FAN CPU. Node internamente
         * tiene sleep dinámicos (Epoll), aquí daremos 1 seg para que veamos los puntitos lentos...
         */
        sleep(1); 
    }

    // ===================================
    // LIMPIEZA FINAL OS DE HIGIENE CIVIL
    // Al ser el STDIN una global del Consola Padre de tu Linux... si la dejas en NON-BLOCK al terminar tu script...
    // ¡Tu Terminal Bash BASH pura madre se bugeará locamente comportándose errática! Devuélvela 
    // su mode formal antes que desbaratar la PC del User!. (Se hace quitandole Bitand Bitwise ~ C)
    // ===================================
    fcntl(STDIN_FILENO, F_SETFL, banderas_viejas);
    printf("\n\n======== HIGIENIZACIONES EFECTUADAS. FD 0 BLOCKING NORMALIZADO. SALIDA SEGURA. ========\n");

    return EXIT_SUCCESS;
}
