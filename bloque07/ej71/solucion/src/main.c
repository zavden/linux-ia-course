/*
 * Ejercicio 7.1 — Cacería de Fantasmas Valgrind (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Provocar Vulnerabilidades Top10 Sys C que un humano es incapaz de notar, 
 * dejando en evidencia cómo la VirtualMachine Memcheck Valgrind los depura
 * bloqueándolo todo protegiendo los deploys Enterprise.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// [!] CASO 1: LEAK (Memoria Perdida) [!]
// No explota, simplemente tu servidor devora un trozo valios de Ram perpetuamente si esta función es llamada en loop.
void forjar_leak_malo() {
    printf("   -> Creando Puntero Vivo y saliendo del Thread local sin Liberarlo Glibc...\n");
    void *secreto = malloc(4096 * 5); // Perdemos 20 KB Puros!
    if(!secreto) return;

    // Basura Ficticia:
    memset(secreto, 'B', 4096*5);
    // Jamás Usamos FREE(). (Olvido C)
}


// [!] CASO 2: USE-AFTER-FREE (Zombies/Hack Exploits) [!]
// Peligrosísimo. Un hacker podría haber metido Shell\_Code_Asm allí antes 
// de que tú tocaras el pointer recién regalado de nuevo y crashear root os!
void forjar_zombie_puntero() {
    printf("   -> Forjando Struct, matándolo... ¡Y tocando su cadaver virtual!\n");

    char *testigo = malloc(64);
    if (!testigo) return;

    strcpy(testigo, "Clave Secreta OS!");
    
    // Asesinato Limpio Glibc. El pedacito ya no nos pertenece, y el Kernell OS y GLIBC
    // puede reasignarlo a otro hilo Pthread en 0.5 nanosegundos!
    free(testigo);  
    
    // ERROR FATAL! Tocar bytes que legalemnte libere!
    // Si tienes suerte, el Glbic no lo ha machacado en la caché todavía e imprimirá normal el Log y el compilador GCC no dice nada! 
    // Magia Negra. Pero en Prod... crasheará un Lunes a las 4AM Rándom. Valgrind te salvará detectando esto!.
    printf("      [VALOR EXCAVADO ZOMBIE ILÍCITO = '%s'] !!!\n", testigo); 

    // O mutarlo despues! (Corrupción Ajena). Valgrind gritará 'Invalid Write of size 1'.
    testigo[0] = 'H'; 
}


// [!] CASO 3: DESBORDAMIENTO (Over-Run Bounds Array) [!]
// C no conoce los Strings ni el Limite. Si creas 5 e intentas mutar el 20...
// Pisa las variables que tenga tu Madre, Kernell OS u Otro programa continuo en la Array Heap!
// Causa SegFaults a veces, pero la Mayoria del tiempo pasa Silencioso pudirendo la matemática contigua!.
void forjar_desbordamiento_ciego() {
    printf("   -> Ciego total. Malloc 10, Escribirmos a Base [15] asincrono C++.\n");

    int *vector = malloc(10 * sizeof(int));
    if (!vector) return;

    for (int i=0; i < 10; ++i) vector[i] = i; 
    
    // OVERFLOW!! No existe la posicion 13.
    // Esto es un OVER-WRITE fuera de limites de Bloque de Page Virtual Cache.
    vector[13] = 99999; 

    // OBER-READ!!. 
    printf("     [LEIDA FUERA DE ARRAY C: %d ]\n", vector[15]);
    
    // Aseo, Valgrind no reportará "Leak", pero SI el Warning "Invalid Read y Write C " a full color!.
    free(vector); 
}


int main(int argc, char *argv[]) {
    printf("========= EMULADOR Y FORJADOR DE BUG Y VULNERABILIDADES KERNEL C =========\n\n");
    if (argc < 2) {
         printf("   [!] Uso: %s <1|2|3>\n", argv[0]);
         printf("       1: Forjar Leaks Fugas FDE Memory\n");
         printf("       2: Forjar Variables Zombie Use-AF\n");
         printf("       3: Limites Desbordados Ram Arr Out Bounds\n");
         return EXIT_FAILURE;
    }

    int hack_id = atoi(argv[1]);

    switch (hack_id) {
        case 1: forjar_leak_malo(); break;
        case 2: forjar_zombie_puntero(); break;
        case 3: forjar_desbordamiento_ciego(); break;
        default: printf("Invalido.\n");
    }

    printf("\n========= EJECUCION GCC CONCLUIDA. (Valgrind será el que Juzgue el desastre OS...) =========\n");

    return EXIT_SUCCESS;
}
