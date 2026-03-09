#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int stock_db = 0; // Secreta Cache Modificada Lentos.
// TODO: pthread_rwlock_t lock = PTHREAD_RWLOCK_INITIALIZER;

void *lector(void *arg) {
    // TODO: loop (ej: 5 vueltas).
    // pthread_rwlock_rdlock(...)
    //   printf(...)
    //   sleep(1);   // !Magia! Aqui verás a otros hilos metiendose y printeando PESE a tu Lock. 
    // pthread_rwlock_unlock
    return NULL;
}

void *escritor(void *arg) {
    // TODO: loop (ej: 5 vueltas).
    // pthread_rwlock_wrlock(...)
    //   stock_db+= 100;
    //   printf("Soy Escritor MUTANDO Todo. NINGUN otro printeo existirá miestras hago este slow sleep...\n")
    //   sleep(1);  // Unico Dios Atrapado... nadie pasará..
    // pthread_rwlock_unlock(...)
    return NULL;
}

int main(void) {
    // Lanza 5 ReadThreads 
    // Lanza 2 Escritores Threads Random 
    // Join Todos. Destruye con pthread_rwlock_destroy(&lock) si queres limpiar tu RAM C++ Heap.
    
    return EXIT_SUCCESS;
}
