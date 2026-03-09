#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define SHM_NAME "/com_shm_test"
#define CAPACIDAD 1024

int main(void) {
    // TODO: shm_open()
    // TODO: ftruncate(fd, CAPACIDAD) para setear límite de la RAM real asignada si era nuevo.
    // TODO: mmap(...) para ligar la variable en C. Char *memoria_viva = mmap(...);

    // TODO: fork()
    // Hijo:
    //      sleep(1); 
    //      printf(memoria_viva); // Escucha voz de la consciencia telepática 
    //      strcpy(memoria_viva, "Hijo muta RAM sin write()"); 
    //      exit
    
    // Padre:
    //      strcpy(memoria_viva, "Datos enormes..."); // Emisor directo. En memoria
    //      wait()
    //      printf(memoria_viva) // Recibió datos del hijo retornados
    //      Aseo y limpieza extrema de FD's y munmap()
    //      shm_unlink(SHM_NAME) Obligatorio para borrar el archivo OS y no saturar /dev/shm

    return EXIT_SUCCESS;
}
