#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

// TODO: sem_t limite_api;

void *solicitud_web(void *arg) {
    // TODO: sem_wait(...)
    //   // ZONA DE PARALELISMO MULTIPLE CONTROLADO
    //   printf("Haciendo select a DB simulando... consumiendo ram\n");
    //   sleep(2);
    // TODO: sem_post(...)
    
    return NULL;
}

int main(void) {
    // TODO: sem_init(&limite_api, 0, 3) (Con 3 licencias max)
    
    // Disparar 10 Treads Peticiones Web a la vez
    // Hacer join de finalizacion
    
    // TODO: sem_destroy(&limite_api) 
    return EXIT_SUCCESS;
}
