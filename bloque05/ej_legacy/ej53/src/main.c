#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int stock_items = 0;
// TODO: Inicializar Mutex: PTHREAD_MUTEX_INITIALIZER;
// TODO: Inicializar CondVar: PTHREAD_COND_INITIALIZER;

void *productor(void *arg) {
    // TODO: Loop x10. 
    // mutx_lock. stock++. cond_signal(o broadcast). unlock.
    // sleep(1); // simula que cuesta producir un carro de la fabrica
    return NULL;
}

void *consumidor(void *arg) {
    // TODO: Loop eterno while(1) {
    // mutex_lock
    // while (stock == 0) { cond_wait(...) duerme con el mutex metido... }
    // stock-- (Come carro)
    // unlock
    return NULL;
}

int main(void) {
    // Lanzar N Productores
    // Lanzar M Consumidores
    // Hacer joined... Ojo, los consumidores tienen loop infinito. Debes pensar una forma de
    // apagar el Flag global de Loop y despertar cond_broadcast un ultimo test de Muerte o cerrar brutal.
    
    return EXIT_SUCCESS;
}
