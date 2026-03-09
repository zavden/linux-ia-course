#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 10000000
#define NUM_THREADS 4

int data_array[SIZE]; // 40 MB de RAM

// TODO: struct thread_args ...

void *worker(void *arg) {
    // TODO: Castear void*. Iterar de start a end, modificando data_array[i]
    return NULL;
}

int main(void) {
    // TODO: Init Threads de for (0 to NUM_THREADS). 
    // Recuerda que cada hilo necesita su propio parametro de struct arg !
    // Ejecutar pthread_create(..)
    
    // TODO: Segundo Loop: pthread_join(...)
    
    // Check de sanidad verificando final.
    // data_array[SIZE-1] y data_array[0] ?

    return EXIT_SUCCESS;
}
