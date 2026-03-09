/*
 * Ejercicio 5.5 — El Pase VIP (Semáforos) (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Introducir las primitivas Posix `sem_t` ilustrando de forma perfecta
 * Cómo capar/detener inundaciones masivas del tipo "Rate Limiter"
 * sin ahogar los descriptores de puertos web del Sistema Operativo de un tirón.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define CONCURRENCIA_MAX 3
#define TSI_ATAQUE 10 // Total Simutaneous Injections 

sem_t vip_door; // Puerta VIP Controladora Del SO Kernell

void *peticion_http_rutina(void *arg) {
    int req_id = *(int*)arg;
    
    printf("   [!] Petición Web #%d Acaba de chocar pidiendo acceso al puerto 80...\n", req_id);
    
    // ============================================
    // BARRERA DE ESPERA SEMAFÓRICA MATEMÁTICA
    // Si la matemática de los tokens cae a 0 o negativo... 
    // OS dormirá este Hilo obligatoriamente ahorrando Ciclos al 0% CPU.
    sem_wait(&vip_door);
    
    // --- ESTAS ADENTRO DEL CLUB VIP ---
    
    // Podemos Consultar cuántos Pases Libres quedaron vivos actualmente dentro simulados:
    int libres = 0;
    sem_getvalue(&vip_door, &libres);
    printf("\n   ====== [DB REQUEST #%d] (Pases Libres en Fila: %d) PROCESANDO PESADO... ======\n", req_id, libres);
    
    // Simulo la Lentitud (Ej, Descargando archivo PDF / Query a BDD Lenta)
    usleep(800000);  // 0.8 s
    
    printf("   <<<<<< [DB REQUEST #%d FINISH] Desalojando Server HTTP C\n", req_id);

    // ============================================
    // RESTAURACIÓN MÁGICA DE TOKEN AUMENTANDO CONTADOR
    // Si hay un zombie afuera en el wait(), Linux lo despertará mágicamente al vuelo inyectándole este Token devuelto.
    sem_post(&vip_door);
    
    return NULL;
}


int main(void) {
    printf("============== RATE LIMITER API C (CAPACITY: %d) ==============\n\n", CONCURRENCIA_MAX);
    
    // Incializando Obsequiando X Monedas/Tokens de pase Libre.
    // El '0' en el parámetro del medio significa que este Semáforo morirá aquí y es Único de este programa Main (No compartido entre Forking C clones Interprocesos).
    if (sem_init(&vip_door, 0, CONCURRENCIA_MAX) != 0) {
        perror("Fatal Fail. Tu Sistema Linux Oslay o kernel no soporta Semáforos Anónimos (Posix falló).");
        return EXIT_FAILURE;
    }

    pthread_t conexiones[TSI_ATAQUE];
    int identificadores[TSI_ATAQUE];

    // Bombardeo Masivo Simulando DOS (Denial of Service) repentino
    for (int i = 0; i < TSI_ATAQUE; i++) {
        identificadores[i] = i + 1;
        pthread_create(&conexiones[i], NULL, peticion_http_rutina, &identificadores[i]);
    }
    
    // El padre Main debe resistir y esperar a todos aunque mueran agrupados de a 3 por turno asínncrono
    for (int i = 0; i < TSI_ATAQUE; i++) {
        pthread_join(conexiones[i], NULL);
    }
    
    // IMPORTANTE: Destructor De Objeto. O dejas Leakeado el Semáforo Poscix en tu RAM (Dependiendo C Lib).
    sem_destroy(&vip_door);
    
    printf("\n========= T0DOS FUERON ATENDIDOS SATISFACTORIAMENTED C POSIX LIMIT =========\n");

    return EXIT_SUCCESS;
}
