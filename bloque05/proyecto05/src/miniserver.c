#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define P_PORT 8080
#define TEAM_HILOS 4

// TODO: Buffer circular Cola de Clientes Aceptados 
// TODO: locks (pthread_mutex_t y cond_t var)

void *worker_hilo(void *arg) {
    // 1. Loop while y cond_wait.
    // 2. Extraer el Socket_fd (Cola de clientes de buffer globlal OS).
    // 3. unlock!
    // 4. read() (Leer petición GET Chrome HTTP texto)
    // 5. write() (Responder 'HTTP/1.1 200 OK' junto a tu codigo crudo / body!)
    // 6. close(socket_fd) (Obligatoiro OS desconectar tcp)
    // 7. Loop de regreso!
    return NULL;
}

int main(void) {
    // 1. Array loop para lanzar los 4 workers inmortalens
    // 2. Setup Sockets (socket, bind(PORT 8080), listen()) OS
    // 3. while(1) Bucle Maestro Paterno:
    //    client_fd = accept(server, ...) // Se Queda Ciego bloqueado y congelado OS esperando Conexiones!
    //    if (client_fd) {
    //         lock();
    //         Insertar al Buffer Circular Cola
    //         signal() o broadcast!
    //         unlock()
    //    }
    
    return EXIT_SUCCESS;
}
