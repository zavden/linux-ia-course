/*
 * Ejercicio 5.4 — RWLocks Caché Multicore (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Demostrar que el Lock de Mutex Normal paralizaría por 5 segundos
 * la simulación "Readers", pero el RWLock C lo colapsa a tan solo
 * 1 Segundo, ya que los 5 entran simultáneamente la sección crítica.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

/* MEMORIA RAM CENTRAL (SIMULADO DB) */
int base_de_datos = 0;

/* LOCKERS DE GESTIÓN OPTIMIZADA DE RENDIMIENTO */
pthread_rwlock_t puente_acceso = PTHREAD_RWLOCK_INITIALIZER;


void *hilo_lector(void *arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 3; i++) {

        // ============================================
        // PIDO PERMISO A LINUX DE LECTURA [READ_LOCK]
        // Si hay un Escritor (Wrlock) activo en la DB. ¡Me Trabó el OS esperándolo!
        // Si hay otros 999120 Lectores (Rdlock) activos en la DB.. ¡El OS me regala verde total 
        // y pasamos de la mano todos de una!
        pthread_rwlock_rdlock(&puente_acceso);
        
        printf("  [ LECTOR %d ] -> Entré asínncronamente en paralelo para Ver Dato Limpio: >> %d <<\n", id, base_de_datos);
        
        /* 
         * DEMOSTRACIÓN DE SIMULTANEIDAD: 
         * Me dormiré pesadamete con LLAVE puesta 1 Segundo simulando un "Select en MySql" Lento. 
         * Si el OS está haciendo bien el RWLock Posix, en tu terminal verás que C imprimen los otros 4 Lectores
         * al mismo momento, ignorando mi Sleep Local Egoista. (Porque ellos pasaron tb!). 
         */
        sleep(1);

        // Suelto Candado Reader
        pthread_rwlock_unlock(&puente_acceso);
        // ============================================
        
        usleep(500000); // 0.5s descanso afuera
    }
    return NULL;
}


void *hilo_escritor(void *arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 3; i++) {

        // ============================================
        // PIDO PERMISO ABSOLUTO DE DESTRUCCIÓN [WRITE_LOCK]
        // Si hay siquiera 1 solo lector tonto o escritor, El OS detendrá mi Hilo.
        // Y cuando yo lo agarre, NADIE pero NADIE NUNCA entrará conmigo.
        pthread_rwlock_wrlock(&puente_acceso);

        printf("\n\033[1;31m  [!] [ESCRITOR %d] -> EL MUNDO SE DETIENE. MUTANDO VARIABLE OS CORE...\033[0m\n", id);
        
        // Mutación Exclusiva
        base_de_datos += 500;
        
        /*
         * Mantenemos Atrapada la BD entera un segundo simulando disco escribiendo!
         * Todo el printlog Reader se Callará de espanto durante esta hora bloqueada
         */
        sleep(1);  
        
        printf("\033[1;31m  [!] [ESCRITOR %d] -> Mutación Terminada. Devuelvo puente a los mortales C++.\033[0m\n\n", id);

        // Devolvemos el poder del universo Kernell
        pthread_rwlock_unlock(&puente_acceso);
        // ============================================

        usleep(1500000); // 1.5s descanso (O los escritores se pisarán constantemente arruinando el test_log visual C).
    }
    return NULL;
}


int main(void) {
    printf("============== CACHE READ/WRITE LOCKS ENGINE C POSIX ==============\n\n");

    pthread_t tb_lectores[5];
    pthread_t tb_escritores[2];
    
    // Arrays para guardar la ID sin corrupciones de Referencias pointers Mutadas de los Threads. 
    int id_lectores[5] = {1, 2, 3, 4, 5};
    int id_escritores[2] = {100, 200};

    // Lanzamiento del Banco Central de Pruebas
    for(int i = 0; i < 5; i++) { pthread_create(&tb_lectores[i], NULL, hilo_lector, &id_lectores[i]); }
    
    // Lanzamiento Atómico de Mutadores Escritores Dios
    for(int i = 0; i < 2; i++) { pthread_create(&tb_escritores[i], NULL, hilo_escritor, &id_escritores[i]); }


    // Esperamos fin en el Padre para no leakear
    for(int i = 0; i < 5; i++) { pthread_join(tb_lectores[i], NULL); }
    for(int i = 0; i < 2; i++) { pthread_join(tb_escritores[i], NULL); }

    printf("\n========= FINALIZADO C C++ CACHE (Total Acumulado Final: %d) =========\n", base_de_datos);

    return EXIT_SUCCESS;
}
