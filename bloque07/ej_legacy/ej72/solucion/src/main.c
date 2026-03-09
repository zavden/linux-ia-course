/*
 * Ejercicio 7.2 — Core Dumps (Morgue Forense C) (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Diseñar bombas lógicas implacables para evaluar la capacidad de 
 * recolección de pruebas post-mortem de GNU gdb.
 */
#include <stdio.h>
#include <stdlib.h>

void recursion_infinita__stack_overflow() {
     // Creará miles de variables en el Stack (No el Heap, el Stack de 8 Megas!) hasta que el 
     // Kernell le corte la puta cabeza por robar límite System.
     volatile char variables_locales_gigantes[1024];
     variables_locales_gigantes[0] = 'X'; 
     recursion_infinita__stack_overflow(); // Se muerde la gola
}

int matar_con_matematica() {
    printf("  -> Calculando Salario Final en base a 110 empleados entre... 0... \n");
    // Volatile prohíbe las optimizaciones Compiler C (-o2, -o3). (Sino gcc diría 'ahh este wey hace 1/0 a propósito, se lo borro y le pongo return 0 para que no muera'.)
    volatile int empleados = 0;
    int presupuesto = 15000;
    
    // BAM! Excepción por Procesador Faltal: (SIGFPE)
    int per_capita = presupuesto / empleados; 
    
    return per_capita;
}

void matar_la_memoria() {
     printf("  -> Hackeando Memoria NULA intocable y protegida C_OS (Dereferenciando Pointer)\n");
     // Pointer Cero! La dirección 0x0000000 es la de Dios (Root OS Protegidisimo Kernell). ¡Si tocas eso y mutas OS C te fulmina del cielo con SIGSEGV!
     char *punto_ciego = NULL; 
     
     // BAM! Muerte Violación de Segmento: (SIGSEGV)
     punto_ciego[0] = 'H';  
     printf("Esto jamas se loggueará:  %c", punto_ciego[0]);
}

int main(int argc, char *argv[]) {
    printf("========= GDB CORE DUMP (FORENSIC TOOLKIT KILLER) =========\n");

    if (argc < 2) {
         printf("   [!] Uso: %s <1|2|3>\n", argv[0]);
         printf("       1: Morir x (Aritmeticha_SIGFPE Processor Trap)\n");
         printf("       2: Morir x (Pointer SEGMENT FAULT SIGSEGV)\n");
         printf("       3: Morir x (Asfixia x Recursividad de OS Limit STACK OVF)\n");
         return EXIT_FAILURE;
    }

    int opcion = atoi(argv[1]);

    switch (opcion) {
        case 1: matar_con_matematica(); break;
        case 2: matar_la_memoria(); break;
        case 3: recursion_infinita__stack_overflow(); break;
        default: printf("Sobreviviste.\n");
    }

    // El return jamás debió lograrse
    return EXIT_SUCCESS;
}
