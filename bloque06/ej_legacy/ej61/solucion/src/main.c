/*
 * Ejercicio 6.1 — Fundamentos de TCP IPv4 (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Abstraer la creacion pura nativa en C Linux de la Pila TCP sin librerias HTTP.
 * Domina el Host-To-Network y la identificación de IP entrantes mediante Accept.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  // inet_ntoa nativa c_os

#define PORT_TCP 9090

int main(void) {
    printf("============ SERVIDOR RAW ECHO TCP EN MODO SINGLE TRHEAD (PORT %d) ============\n", PORT_TCP);

    // ===================================
    // 1. ANATOMÍA C++ DEL ENDPOINT
    // ===================================

    // Domain AF_INET (IPv4), Type SOCK_STREAM (Confiable, Sin Pérdida bytes: = TCP !).
    // Si quisieras UDP cambiarías stream a SOCK_DGRAM (Para videojuegos Mando o Skype).
    int s_master = socket(AF_INET, SOCK_STREAM, 0);
    if (s_master < 0) {
        perror("Falla OS de asignación kernel a nuevo socket");
        return EXIT_FAILURE;
    }

    // Prevencion de bloqueo REUSE_ADDR "Bind error port already in use" comun en linux C C++
    int t = 1; setsockopt(s_master, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(int));

    // ===================================
    // 2. MATRIZ DE DIRECCIONAMIENTO ARPA
    // ===================================
    struct sockaddr_in dir_mia;
    // Si no le limpias la basura de la struct (bzero / memset 0) crasheará en tu Linux extrañísimamente despues.
    memset(&dir_mia, 0, sizeof(dir_mia)); 

    dir_mia.sin_family = AF_INET;
    // INADDR_ANY -> (0.0.0.0). "Oye OS Kernell dame mi app por tarjeta de RED Local LAN interna 192.. y por la loopback 127 local".
    dir_mia.sin_addr.s_addr = htonl(INADDR_ANY); 
    
    // CPU Normal (Little Endian). CPU de Switches / Redes Cables Mágicos (Big Endian). 
    // `htons()` (Host To Network Short) convierte tu puerto C 9090 y lo "vuelca" del derecho a la inversa bit a bit
    // De lo contrario tratarás de escuchar por el puerto 8355 en lugar de 9090.
    dir_mia.sin_port = htons(PORT_TCP);

    // ===================================
    // 3. REGISTRANDO EL NEGOCIO (BIND Y LISTEN)
    // ===================================
    if (bind(s_master, (struct sockaddr *)&dir_mia, sizeof(dir_mia)) < 0) {
        perror("OS Fatal Bind_Error"); return EXIT_FAILURE;
    }
    
    // Antenas Vivas Encendidas OS con Cola 10 (Somaxconn límite default Kernel)
    if (listen(s_master, 10) < 0) {
         perror("OS Error iniciando escuchas de Listen"); return EXIT_FAILURE;
    }

    // ===================================
    // 4. EL EVENT LOOP SINCRONICO (Mortalidad a gran Escala)
    // ===================================
    printf("\n  >> Kernel OS Atento. Abre otro bash y Ejecuta: 'nc 127.0.0.1 %d'\n", PORT_TCP);

    while (1) {
        struct sockaddr_in hacker_remoto;
        socklen_t s_len = sizeof(hacker_remoto);

        // Bloqueo duro! Nos congelaremos por horas aquí al %0 CPU OS Kernell si nadie ataca 
        int target_fd = accept(s_master, (struct sockaddr *)&hacker_remoto, &s_len);
        if (target_fd < 0) continue; // Si failió, vuelve y perdona rapido a tratar C.

        // MÁGIA C DE PARSEO INVERSO: Desempaqueta y Vuelca lo Entrante Big Endian y convierte Ip Int pura en String 192.x !
        char *ip_extranjera = inet_ntoa(hacker_remoto.sin_addr);
        int ip_puerto_ext = ntohs(hacker_remoto.sin_port);

        printf("\n\033[1;36m[+] Intruso Connectado Exitósamente! ->  %s:%d\033[0m\n", ip_extranjera, ip_puerto_ext);
        
        // Escribiéndome Atómicamente el Greeting C 
        const char *w_msg = " [!] Has ingresado de Forma exitósa al Server Test de C Sockets. Escribe algo...\n";
        write(target_fd, w_msg, strlen(w_msg));

        // 5. CAUTIVERIO SECUENCIAL (El Lado Malo)
        // Todo cliente NUEVO que llame accept... SE VA A QUEDAR ESPERANDO a que este que está acá adentro termine 
        // de chatear y se cierre... Por esto se hizo Node Js Asínncrono Epoll y Nginx!!. Nadie deberia quedarse atrapado infinitamente en 
        // Single Thread asi o matarás las apps concurrentes del publico por starvation C de CPU I/O !
        char buf[128];
        ssize_t bytes_c;

        while ( (bytes_c = read(target_fd, buf, sizeof(buf) - 1)) > 0 ) {
            buf[bytes_c] = '\0'; 
            printf("          => Mensaje Intruso Recibido: %s", buf);

            if (strncmp(buf, "quit", 4) == 0) {
                 printf("\n          => Desconexión Remota pacífica Acatada.\n");
                 break;
            }
        }

        if (bytes_c == 0) {
             printf("   [-] Desconexión Fuerte Asíncrona (Aprestaste CRTL C desde tu cliente NC! OS Disconnected).\n");
        } 

        // Obligatorio no leakear File Descriptors Limits (Límite 1024 FD)
        close(target_fd);
        printf("\033[1;31m[-] Sesión OS del Target %s clausurada y Limpia de la memoria RAM OS C\033[0m\n", ip_extranjera);
    }

    close(s_master);
    return EXIT_SUCCESS;
}
