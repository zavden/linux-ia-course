#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PUERTO 9090

int main(void) {
    // 1. Crear Socket AF_INET SOCK_STREAM.
    // 2. Struct sockaddr_in con INADDR_ANY y htons(PUERTO).
    // 3. bind()
    // 4. listen(max_queue)
    
    // while(1) Bucle de red
        // int client_fd = accept(...)
        // if() // Validar!
        // printear IP intrusa
        
        // char buffer[1024];
        // write(client_fd, "Hola intruso", 12);
        
        // Bucle interno de lectura
        // while ( (n = read(client_fd, buffer, ...)) > 0 )  
             // Echo al server 
             
        // close(client_fd)
        // Bucle Regresa a la siguiente persona accept!

    return EXIT_SUCCESS;
}
