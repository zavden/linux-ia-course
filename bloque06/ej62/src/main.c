#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main(void) {
    // 1. Convertir 0 (STDIN) a O_NONBLOCK.
    //      int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    //      fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    
    // 2. Loop Asincrono Central (El papa de todo Async/Await e EventLoops).
    printf("Inicio de Node-Like OS Polling (Escribe 'quit' + Enter para escapar)...\n");
    // while (1) {
    //     char buffer[64];
    //     ssize_t leidos = read(STDIN_FILENO, buffer, sizeof(buffer)-1);
        
    //     if (leidos > 0) { ... Print...  }
    //     else if (leidos < 0) {
    //         if (errno == EAGAIN || errno == EWOULDBLOCK) {
    //              // MAGIA: El read nos rechazo pacíficamente sin bloquear C !!
    //              // Pone solo un puntito (.) en la consola simulando que tu aplicacion hace otras cosas (mueve graficos, juegos)
    //         } 
    //     }
        
    //     sleep(1); // Simulando Ticks/Frames lentos de tu juego o CPU App (Evita que el While queme a 100% Top Htop en vacio)
    // }

    // (Opcional): Si saliste por quit.. deberias regresar el STDIN_FILENO a Blocking quitando el flags & ~O_NONBLOCK 
    // o arruinarias el bash de tu master OS general Linux
    
    return EXIT_SUCCESS;
}
