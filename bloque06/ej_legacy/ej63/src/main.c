#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
// #include <sys/epoll.h>  -> MÁGIA C DE MULTIPLEXACION KERNELL!

#define PORT 8080
#define MAX_EVENTS 64

// TODO: Funcion de ayuda 'set_nonblocking(int fd)' que inyecte O_NONBLOCK asincrona a FDs.

int main(void) {
    // 1. Array clasico Server TCP de (socket, bind, listen) PUERTO 8080! -> "server_fd"
    // 2. set_nonblocking(server_fd); !! ESTO SALVA TU SERVIDOR C_OS TCP DE DEMONIOS MUERTOS.
    
    // int epoll_fd = epoll_create1(0);
    
    // struct epoll_event event_setup;
    // event_setup.data.fd = server_fd;
    // event_setup.events = EPOLLIN | EPOLLET; // Edge-Triggered / O Escucha Continua.
    // epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event_setup)
    
    // struct epoll_event *notificaciones_vivas = calloc(MAX_EVENTS, sizeof(struct epoll_event));

    // while (1) {
    //    // El Nucleo de Nodos. (El 100% de NodeJS funciona aqui de fondo!!)
    //    int activaciones = epoll_wait(epoll_fd, notificaciones_vivas, MAX_EVENTS, -1);
       
    //    for (int i=0; i < activaciones; i++) {
    //         int socket_responsable = notificaciones_vivas[i].data.fd;
            
    //         if (socket_responsable == server_fd) {
    //             // HEY! FUE EL HOST MASTER DE RED.
    //             // Alguien hizo un ping a nuestra WiFi intentando una Conexion TCP nueva!!
    //             // accept() al nuevo. Y agregarlo de igual manera al epoll_ctl ADD. (¡TAMBIEN HAZ AL NUEVO set_nonblocking()!) 
    //         } else {
    //             // FUE UN USUARIO ORDINARIO YA CONECTADO PREVIAMENTE ! (¡ESCRIBIÓ ALGO!)
    //             // read()
    //             // ! Ojo ! Trata EAGAIN si programaste en EPOLLET.
    //             // Si mando datos -> Imprimir C
    //             // Si read dio 0 -> El men nos Cerro la pestaña del netcat_cliente. (close(fd)) ¡Epoll lo borra automático feliz C!
    //         }
    //    }
    // }
    
    return EXIT_SUCCESS;
}
