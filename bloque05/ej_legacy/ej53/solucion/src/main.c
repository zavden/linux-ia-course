/*
 * Ejercicio 5.3 — Variables de Condición y Broadcasts (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Prevenir la muerte térmica de un CPU por Pooling agresivo (While_True_If_True).
 * Cond_Wait descansa en la API de OS (FUTEXES de Linux) que lo duermen,
 * liberándole el hilo de núcleo mágico al procesador general perdiendo control 
 * hasta que literalmente otro hilo avisa `wake!` de hardware.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/* VARIABLES COMPARTIDAS MAGNAS */
int stock_buffer = 0;
int max_comidas = 15; // Límite artificial para que esto no corra al infinito C
int comidas_consumidas = 0;

/* CONTROLES POSIX */
pthread_mutex_t m_global = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv_stock  = PTHREAD_COND_INITIALIZER;


void *hilo_productor(void *arg) {
    int id = *(int*)arg;
    for (int i = 0; i < max_comidas; i++) {
        // Pseudo-Simulación: "Preparar un platillo me toma 0.5 segs..."
        usleep(500000); // 0.5 seg

        pthread_mutex_lock(&m_global);
        
        // == ZONA CRÍTICA Y AISLADA ==
        stock_buffer++;
        printf("   [+] Productor Chef %d Coció Plato Rápido! Olla Común = %d C/Stock\n", id, stock_buffer);
        
        // La campana Milagrosa.
        // Diferencia: `cond_signal` Despierta solo a UNO de los que esté dormido (Linux Random).
        // `cond_broadcast` Despierta a los TRES a la vez causándoles un pleito por ver quien hace Lock primero. 
        pthread_cond_broadcast(&cv_stock);
        // ============================

        pthread_mutex_unlock(&m_global);
    }
    printf("\t>>>[+] Productor Jefe de Cocina terminó todo su trabajo por hoy!\n");
    return NULL;
}

void *hilo_consumidor(void *arg) {
    int id_hambre = *(int*)arg;

    while (1) {
        pthread_mutex_lock(&m_global);

        // COND_WAIT NECESITA UN WHILE Y NO UN IF (The Spurious Wakeup OS-Level Bug Rule).
        // Hay una posibilidad estadísticametne comprobada de que Linux despierte tu cond_wait
        // ¡Por culpa de interrupciones asincronas de OS Systemas ajenos a tu C!.
        // Si despierta y el plato sigue  en == 0, se volverá a asfixiar volviendo al wait!.
        while (stock_buffer == 0 && comidas_consumidas < max_comidas) {
             printf(" [C-%d] Búfer al 0 Stock. Durmiéndome ciegamente cediendo Núcleo a 0%% CPU esperando cond_broadcast...\n", id_hambre);
             
             // ALTO AQUÍ: 
             // Nos dormimos. PERO estamos con llave de MUTEX_LOCK metida adentro ¿Verdad?
             // ¡Si nos quedamos dormidos con la llave, el Chef NUNCA POdría entrar a cocinas a hacer stock++ y habría Deadlock!
             // `pthread_cond_wait()` Es una mágica función que Internamente: Te ARRANCA la llave de las manos en Microsegundos
             // y a la vez te apaga a estado sleep(). Cuando te despíerta, automáticamente hace un Relock para tu línea de abajo. 
             pthread_cond_wait(&cv_stock, &m_global);
        }

        // --- Alguien tocó la campana, y SI fue verdaderamente por existencia de stock C! ---
        
        // Criterio de abandono pacífico Muerte Lógica.
        if (comidas_consumidas >= max_comidas) {
             printf(" [C-%d] Se acabó el Día. Me retiro de cocinas con estomago lleno.\n", id_hambre);
             pthread_mutex_unlock(&m_global);
             break; // Rompo while_1
        }
        
        // Acción Vital 
        stock_buffer--;
        comidas_consumidas++;
        printf(" [-] [C-%d] Yo, Consumidor Feroz, Robé un plato!. Quedan en mesa = %d.\n", id_hambre, stock_buffer);

        pthread_mutex_unlock(&m_global);

        // "Me cuesta comer masticar..."
        usleep(100000); 
    }
    return NULL; // Se extrae pacíficamente y muere este Worker Kernel OS.
}

int main(void) {
    printf("==== EL CONGRESO DE ZOMBIES COMEDORES Y EL CHEF ====\n\n");

    pthread_t tb_productor;
    pthread_t tb_consumidores[3];
    int params_c[3] = {1, 2, 3};
    int id_p = 100;

    // Instancia el Generador Activo
    pthread_create(&tb_productor, NULL, hilo_productor, &id_p);

    // 3 Generadores Pasivos Muertos Hámbre (Si el Cpu gasta el 100% acá hay un bug, debe no gastar nada en el wait)
    for (int i = 0; i < 3; i++) {
        pthread_create(&tb_consumidores[i], NULL, hilo_consumidor, &params_c[i]);
    }

    // El join del final para evitar Memory Leaks / Segfault de OS
    pthread_join(tb_productor, NULL);

    /* 
     * MANIOBRA FINAL DEL ABANDONO:
     * Si el ultimo Productor se fue. Los 3 consumidores podrían haberse quedado atorados eternamente
     * en el `cond_wait` adentro creyendo que un día habría platos nuevos!. 
     * Simulamos un "Campanazo Final de Cierre Total del Restaurante" para despabilar el while de la muerte 
     * a los 3 y que por fin pasen al IF limit que dictó break; del while1 general.
     */
    pthread_mutex_lock(&m_global);
    // comidas_consumidas ya está en tope artificial y lo notarían
    pthread_cond_broadcast(&cv_stock);
    pthread_mutex_unlock(&m_global);

    // Recolectar a los hijos
    for (int i = 0; i < 3; i++) { pthread_join(tb_consumidores[i], NULL); }
    
    printf("\n   ====== [SIMULACIÓN TERMINADA. TODO EL RESTAURANTE FUE CERRADO (Leaks/Deadlocks=0)] ======\n");

    return EXIT_SUCCESS;
}
