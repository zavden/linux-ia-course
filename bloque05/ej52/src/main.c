#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

long long target_inseguro = 0;
long long target_seguro = 0;

// TODO: Instancia un pthread_mutex_t = PTHREAD_MUTEX_INITIALIZER;

void *worker_peligroso(void *arg) {
    // TODO: Loop 1 Millón iteraciones 
    // Que sume: target_inseguro++  (Provocara un Desastre C de Choque de trenes)
    return NULL;
}

void *worker_blindado(void *arg) {
    // TODO: Loop 1 Millón iteraciones
    // TODO: mutex_lock(...)
    //       target_seguro++ 
    // TODO: mutex_unlock(...)
    return NULL;
}

int main(void) {
    // 1. Lanzar 10 Treads Peligrosos y hacer sus JOIN.
    // 2. Imprimir. "Debía ser 10,000,000 pero resultó..." ?
    
    // 3. Lanzar 10 Threads Blindados y hacer sus Join...
    // 4. Imprimir. Verás su número 10 Millones limpiezísimo a costa de Time Limit lentitud.

    return EXIT_SUCCESS;
}
