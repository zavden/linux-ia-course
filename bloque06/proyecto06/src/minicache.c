#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
// #include <sys/epoll.h> // Kernell

#define PORT 9595
// Estructura Data
typedef struct {
    char key[32];
    char val[128];
    int active;
} k_store;

k_store db_memory[100]; // 100 espacion C RAM

// Funcion Protocolo String SET -> return OK
// Funcion Protocolo String GET -> Return val
// Funcion set_nonblocking(fd) -> fcntl();

int main(void) {
    // 1. Socket, bind, listen() -> server_fd.
    // 2. set_nonblocking(server_fd);
    
    // 3. epoll_create1(0); 
    // 4. epoll_ctl(.. ADD server_fd ..)
    
    // while (1) {
    //     epoll_wait()
    //     for 0 to ready : {
    //          if (este es == server_fd) accept(), nonblock(nuevo_fd), epoll_ctl(ADD nuevo_fd)
    //          else {
    //               // Chat
    //               while (read() > 0) { 
    //                     // Parsear la String "SET edad 25" \n
    //                     // Aplicar Funcion Logica.. Responder C_TCP! write(client) "OK.."
    //               }
    //               // Si read regresa 0 -> close(client) Borrar FD
    //          }
    //     }
    // }

    return EXIT_SUCCESS;
}
