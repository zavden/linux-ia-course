/*
 * Ejercicio 5.2 — Choque de Trenes (Mutex) (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Empiricamente generar un Crash asincrónico por Race Condition 
 * Desmitificando que la instruccion `a++` de c es Atómica (No lo es en ASM OS).
 * Y arreglarlo introduciendo a la Pobreza y Demora colosal de los Mutex (Semáforos Bloqueantes).
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N_THREADS 10
#define VUELTAS_MONETARIAS 1000000 // 1 Millon de depositos por cada uno

long long saldo_corrupto = 0;
long long saldo_bancario = 0;

/* EL CERROJO MAESTRO */
// Es una estructura pre-compilada C posix global en candado abierto.
pthread_mutex_t candado_cajero = PTHREAD_MUTEX_INITIALIZER;


/* 
 * EL DESTRUCTOR
 * Todos atacarán la variable bruta a la vez cruzando sus instrucciones ASM 
 * LOad-ADD-Store entre sí milésimas antes que el otro la guarde machacando info.
 */
void* roba_monedas_race(__attribute__((unused)) void *arg) {
    for (int i = 0; i < VUELTAS_MONETARIAS; i++) {
        saldo_corrupto++;
    }
    return NULL;
}

/* 
 * EL CUSTODIO DESTRUCTOR DE RENDIMIENTO PERO SALVADOR DE VIDAS
 */
void* cajero_ordenado(__attribute__((unused)) void *arg) {
    for (int i = 0; i < VUELTAS_MONETARIAS; i++) {
        
        // El Hilo 2 hace lock. Cerró la puerta. Si el Hilo 5 entra buscando sumar...
        // ¡Al intentar leer Lock será CONGELADO por orden del Sistema Operativo en Pausa! (Ahorrando en parte CPU).
        pthread_mutex_lock(&candado_cajero);
        
        // ==== SECCIÓN CRÍTICA DE AISLAMIENTO ABSOLUTO (1 PERSONA VIP POR VEZ) ====
        saldo_bancario++;
        // =========================================================================

        // El Hilo 2 desbloquea, y Linux instantáneamente "Despierta" al competidor trabado pasándole turno al instante.
        pthread_mutex_unlock(&candado_cajero);
    }
    /**
     * NOTA SENIOR: Esta solucion es de escuela pero PESIMA INGENIERÍA REAL. 
     * Si cada hilo tiene que pedir "Permiso global al OS -> bloquear a 9 amigos -> Hacer 1 simple puto ++ -> soltar permiso global" 
     * el tiempo se volverá astronómico y mil veces peor que poner 1 miserable HILO Secuencial normal!
     * 
     * (El Buen Patrón Oculto acá sería: Que cada hilo sumara `var_local_sola` un millón de veces sin Lock rápido y veloz en RAM...
     * ¡Y que SOLO HAGAN 1 LOCK GLOBAL AL FINAL ANTES DEL RETURN SUMANDO de golpe el Array subyacente de Output general!)
     */
    return NULL;
}

int main(void) {
    pthread_t ths_malos[N_THREADS];
    pthread_t ths_buenos[N_THREADS];
    
    printf("===== PREVISTO MATEMÁTICO UNIVERSAL: %lld =====\n\n", (long long)N_THREADS * VUELTAS_MONETARIAS);

    // ===================================
    // RONDA 1: FUGAS POR DATA RACES (ANARQUÍA C++)
    // ===================================
    printf("[*] DISPARANDO 10 HILOS INSEGUROS DE A 1 MILLON CADA UNO AL MISMO BLANCO...\n");
    clock_t i_m = clock();
    
    for (int i = 0; i < N_THREADS; i++) {
        pthread_create(&ths_malos[i], NULL, roba_monedas_race, NULL);
    }
    for (int i = 0; i < N_THREADS; i++) { pthread_join(ths_malos[i], NULL); }
    
    clock_t f_m = clock();
    
    printf("   [!] Tiempo tomado: %f segs. Rapisísimo.\n", (double)(f_m - i_m)/CLOCKS_PER_SEC);
    printf("   [!] Saldo Bancario Resultante: \033[1;31m%lld\033[0m  <<< ¡¡FATALIDAD! Data Race Destruyó Millones en colisiones Múltiples ASM!!\n\n", saldo_corrupto);

    // ===================================
    // RONDA 2: PROTECCIÓN SYNCHRONOUS MUTEX 
    // ===================================
    printf("[*] DISPARANDO 10 HILOS BLINDADOS A LOCKS MUTEX EN SECCIÓN CRÍTICA...\n");
    i_m = clock();

    for (int i = 0; i < N_THREADS; i++) {
        pthread_create(&ths_buenos[i], NULL, cajero_ordenado, NULL);
    }
    for (int i = 0; i < N_THREADS; i++) { pthread_join(ths_buenos[i], NULL); }
    
    f_m = clock();
    
    printf("   [!] Tiempo tomado: %f segs  <<< (Lento! Lógica de fila de cajero único OS!).\n", (double)(f_m - i_m)/CLOCKS_PER_SEC);
    if(saldo_bancario == 10000000){
      printf("   [!] Saldo Bancario C Depositado:\033[1;32m %lld\033[0m <<< (PERFECCIÓN Thread-Safe! Ningún byte perdido).\n\n", saldo_bancario);
    } else {
         printf("   [X] Saldo Bancario C Depositado: %lld \n", saldo_bancario);
    }

    return EXIT_SUCCESS;
}
