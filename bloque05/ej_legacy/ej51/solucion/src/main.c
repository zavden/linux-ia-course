/*
 * Ejercicio 5.1 — The Matrix (Pthreads Básicos) (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Introducir concurrencia masiva real.
 * Superar la barrera de "1 sólo argumento nulo" empacando paquetes de punteros 
 * a la fuerza en Heap para envíos de tareas seguros C++.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <unistd.h>
#include <time.h>

#define SIZE_DATA 10000000 // 10 Millilitos.
#define NUM_THREADS 4

/*
 * REGLA DE ORO GLOBAL: Toda concurrencia vive a través de estructuras puntero.
 */
typedef struct {
    int thread_id;
    int index_start;
    int index_end;
    double *array_target; 
} box_tarea_t;

// Variable global sin inicializar BSS, (El OS linux garantiza arrancar Arrays Gigantes Globales limpios en Ceros puros).
double data_global[SIZE_DATA];


/*
 * EL ESCLAVO
 * Esta función *Diferenciará* su comportamiento leyendo su misión descifrada
 * de su caja de correo personal envíada en Ram desde el Main.
 */
void *worker_node(void *paquete_mision) {
    // 1. Resucitando el paquete anónimo (Cast Fuerte Puntero)
    box_tarea_t *mision = (box_tarea_t *)paquete_mision;

    // Solo un print para debugear asíncrono
    printf("   [T#%d] Aterrizó vivo. Segmento encomendado: %d  Al  %d\n", mision->thread_id, mision->index_start, mision->index_end);

    // 2. PARALELISMO MATEMÁTICO REAL 
    // Ningún hilo se tocará la matriz mutuamente (Misión particionada sin Race Conditions)!
    for (int i = mision->index_start; i < mision->index_end; ++i) {
        // Cálculo tonto forzoso computacional pesado que queme Clock de CPUs artificialmente.
        double pseudo_v = (double)i * 3.14159f / 2.7f;
        mision->array_target[i] = pseudo_v + (double)mision->thread_id; 
    }

    return NULL; // Se extrae pacíficamente y muere este Worker Kernel OS.
}

int main(void) {
    printf("======== TEST A: Procesamiento Secuencial Viejo (1 Hilo Main) ========\n");
    clock_t s_inicio = clock();
    
    // Lo hace todo el papá solito y abandonado. (Tarda bastantes MS/Clocks)
    for (int i = 0; i < SIZE_DATA; ++i) {
        double pseudo_v = (double)i * 3.14159f / 2.7f;
        data_global[i] = pseudo_v + 99.0; // Pongo 99 por diferenciar Main.
    }
    
    clock_t s_fin = clock();
    double old_tiempo = (double)(s_fin - s_inicio) / CLOCKS_PER_SEC;
    printf("        Demora Secuencial Total = %f Secs. \n\n", old_tiempo);

    
    printf("======== TEST B: Multiverso Activado (PTHREADS = %d) ========\n", NUM_THREADS);
    // Reiniciamos un toque la memoria 
    for(int i = 0; i < SIZE_DATA; ++i) { data_global[i] = 0.0; }

    // Arrays administradores Locales C.
    pthread_t thread_obj[NUM_THREADS];
    box_tarea_t misiones[NUM_THREADS]; // Estructuras de argumentos

    int tramo_partida = SIZE_DATA / NUM_THREADS;
    
    // ===================================
    // 1. EL BOMBARDEO DE CREACIONES DEL CLÚSTER
    // ===================================
    // IMPORTANTE ACÁ LA VENTAJA FRENTE A CLOCK() DE C TRADICIONAL, 
    // Usaremos timespec reloj de pared REAL POSIX para no sumar Clocks Multicore de Linux que arrojarían mal el bench.
    struct timespec _ti, _tf;
    clock_gettime(CLOCK_MONOTONIC, &_ti);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        // Llenando la encomienda
        misiones[i].thread_id = i;
        misiones[i].index_start = i * tramo_partida;
        // Prevenir asimetría si la división C no fue exacta 0 en modulo (El último barre el remanente total).
        misiones[i].index_end = (i == NUM_THREADS - 1) ? SIZE_DATA : (i + 1) * tramo_partida;
        misiones[i].array_target = data_global;

        // INYECCIÓN MÁGICA 
        // Pasamos por REFERENCIA `&misiones[i]` para no corromper ni colisionar los args a los otros
        int code = pthread_create(&thread_obj[i], NULL, worker_node, &misiones[i]);
        if (code != 0) {
            perror("Error descontrol de OS (Limits de Multihilos excedido / C++)");
            exit(EXIT_FAILURE);
        }
    }

    // ===================================
    // 2. EL BUCLE DE CONTENCIÓN C++ / JOIN()
    // ===================================
    // A este punto del Universo, hay 5 Main loops paralelos en tu Arquitectura de Motherboard!.
    // Main se clava aquí en estado zombie congelándose de a 1 pacíficamente hasta que los 4 lleguen y hagan su `return NULL`.
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(thread_obj[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &_tf);
    double m_tiempo = (_tf.tv_sec - _ti.tv_sec) + (_tf.tv_nsec - _ti.tv_nsec) / 1e9;
    
    printf("\n        Demora PTHREADS Multithread Real OS = %f Secs. \n", m_tiempo);


    // ===================================
    // 3. ASEGURAMIENTO 
    // ===================================
    int correctos = 0;
    // Comprobamos la muestra base del multiloop final:
    // La suma del id: El ultimo Hilo de thread_id 3 debio forzosamente sumar su 3.0 al final del Array Index [9 Milloness]
    if (data_global[SIZE_DATA - 1] > 3.0) correctos = 1;

    if (m_tiempo < old_tiempo && correctos) {
         printf("Veredicto Empático: WIN. Multithreading arrasó exitosamente, Mutó la Array C y devolvió.\n");
    }

    return EXIT_SUCCESS;
}
