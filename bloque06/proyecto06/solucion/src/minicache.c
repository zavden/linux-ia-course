/*
 * Proyecto 6 — minicache (Redis/Memcached Clon) (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Combinar C puro con el I/O Asíncrono Non-Block de Epoll creando 
 * una Base de Datos LRAM Mem_Cache Inmortal que evada Pthreads para
 * evitar bloqueos de Mutexes lentos. O(1) Escabilidad Mundial OS.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>

#define CACHE_PORT 9595
#define CACHE_MAX_ENTS 1024
#define MAX_EPOLL_EV 64

/* ========================================================
 * RAM KEY-VALUE STORE (No Mutexes Needed! Todo es 1Hilo Master Loop)
 * ======================================================== */
typedef struct {
    char key[32];
    char value[128];
    int ok_taken;  // Flag ocupado bit OS
} KVal;

KVal my_db[CACHE_MAX_ENTS]; // El espacio Memoria Ram (40KB - 50KB ligerito Kernell)

/*
 * HELPERS: NON-BLOCKING 
 */
int asignar_super_velocidad_nonblocking(int fd_socket) {
    int blz_flags = fcntl(fd_socket, F_GETFL, 0);
    if (blz_flags == -1) return -1;
    return fcntl(fd_socket, F_SETFL, blz_flags | O_NONBLOCK);
}

/*
 * PARSER DE REDIS PROTOCOL 
 */
void handler_parseo_db(int target_cliente_fd, const char *comando_crudo) {
    // Eliminando los mugrosos enters telnet de windows / linux (\r \n) C
    char linea[256];
    strncpy(linea, comando_crudo, sizeof(linea)-1);
    linea[sizeof(linea)-1] = '\0';
    for (int i=0; linea[i]; i++) { 
        if (linea[i] == '\r' || linea[i] == '\n') { linea[i] = '\0'; break; } 
    }

    // "Variables receptoras" C-OS
    char o_cmd[16] = {0}, o_key[32] = {0}, o_val[128] = {0};

    // MAGIA: Usamos sscanf inteligente para separar por espacios!
    // %s = 1 Palabra. %127[^\n] = Todo un String Completo con espacios para el "Valor".
    int match_elementos = sscanf(linea, "%15s %31s %127[^\n]", o_cmd, o_key, o_val);
    
    char w_buffer[256];

    // ================= [ COMANDO GET ] =================
    if (strncmp(o_cmd, "GET", 3) == 0 && match_elementos >= 2) {
        int e_status = 0;
        for (int i = 0; i < CACHE_MAX_ENTS; i++) {
            if (my_db[i].ok_taken && strncmp(my_db[i].key, o_key, 32)==0) {
                 snprintf(w_buffer, sizeof(w_buffer), "VALUE: %s\n", my_db[i].value);
                 write(target_cliente_fd, w_buffer, strlen(w_buffer));
                 e_status = 1; 
                 break;
            }
        }
        if (!e_status) write(target_cliente_fd, "NULL\n", 5);
    } 
    // ================= [ COMANDO SET ] =================
    else if (strncmp(o_cmd, "SET", 3) == 0 && match_elementos >= 3) {
         int libre_id = -1;
         int status_ok = 0;
         
         // Prioridad 1: Actualizar Existente si ya estaba.
         for (int i = 0; i < CACHE_MAX_ENTS; i++) {
            if (my_db[i].ok_taken && strncmp(my_db[i].key, o_key, 32)==0) {
                 strncpy(my_db[i].value, o_val, 128); status_ok = 1; break;
            }
            if (!my_db[i].ok_taken && libre_id == -1) libre_id = i; // Me guardo un índice de memoria libre por sin no existía!
         }
         
         // Prioridad 2: No Existía la llave. Forjamos el Registro C_TCP Nuevo!.
         if (!status_ok && libre_id != -1) {
             my_db[libre_id].ok_taken = 1;
             strncpy(my_db[libre_id].key, o_key, 32);
             strncpy(my_db[libre_id].value, o_val, 128);
             status_ok = 1;
         }
         
         if (status_ok) write(target_cliente_fd, "OK\n", 3);
         else write(target_cliente_fd, "ERROR_OOM\n", 10);
    } 
    // ================= [ COMANDO DEL ] =================
    else if (strncmp(o_cmd, "DEL", 3) == 0 && match_elementos >= 2) {
         int b_status = 0;
         for (int i = 0; i < CACHE_MAX_ENTS; i++) {
            if (my_db[i].ok_taken && strncmp(my_db[i].key, o_key, 32)==0) {
                 my_db[i].ok_taken = 0; // Marcar basura recolectora OS!
                 write(target_cliente_fd, "OK\n", 3);
                 b_status = 1; break;
            }
         }
         if (!b_status) write(target_cliente_fd, "NULL\n", 5);
    } 
    else {
        // Enrutando un mal query humano a pantalla C_TCP Cliente.
         write(target_cliente_fd, "-ERR Unknown CMD Protocol\n", 26);
    }
}


/* ========================================================
 * EL MOTOR V8 / EVENT LOOP MASTER DE EPOLL TCP REDES C
 * ======================================================== */
int main(void) {
    printf("============== MINICACHE C REDIS-LIKE SERVER (%d) ==============\n", CACHE_PORT);
    // Vaciamos Memoria Pura RAM por si algo malo del Glibc Kernel
    memset(my_db, 0, sizeof(my_db));

    int tcp_master = socket(AF_INET, SOCK_STREAM, 0);
    int opc = 1; setsockopt(tcp_master, SOL_SOCKET, SO_REUSEADDR, &opc, sizeof(opc));

    struct sockaddr_in mis_reglas;
    memset(&mis_reglas, 0, sizeof(mis_reglas));
    mis_reglas.sin_family = AF_INET;
    mis_reglas.sin_addr.s_addr = INADDR_ANY; 
    mis_reglas.sin_port = htons(CACHE_PORT);

    bind(tcp_master, (struct sockaddr *)&mis_reglas, sizeof(mis_reglas));
    asignar_super_velocidad_nonblocking(tcp_master);
    listen(tcp_master, SOMAXCONN);

    // MATRIX_KERNLL_MÁGICO (EPOLL)
    int cerebro_epoll = epoll_create1(0);

    struct epoll_event ctl_antena_master;
    ctl_antena_master.data.fd = tcp_master;  
    ctl_antena_master.events = EPOLLIN | EPOLLET; // Input EdgeTrigger C  
    epoll_ctl(cerebro_epoll, EPOLL_CTL_ADD, tcp_master, &ctl_antena_master);

    struct epoll_event campanas_os[MAX_EPOLL_EV];

    printf("  [+] Cache Database Iniciada. Conéctate con NETCAT en otro bash tcp: `nc 127.0.0.1 %d`\n\n", CACHE_PORT);

    // ===================================
    // EL EVENT LOOP MAESTRO ETERNO
    // ===================================
    while (1) {
        int despiertos = epoll_wait(cerebro_epoll, campanas_os, MAX_EPOLL_EV, -1);
        
        for (int i = 0; i < despiertos; i++) {
            
            // a) ¿PAPÁ_MASTER SE MOVIÓ? (Si sí: Request C_Accept IP Infiltrada nueva)
            if (tcp_master == campanas_os[i].data.fd) {
                while (1) {
                    struct sockaddr dir_esclava;
                    socklen_t in_len = sizeof(dir_esclava);
                    int infiltrado = accept(tcp_master, &dir_esclava, &in_len);
                    
                    if (infiltrado == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break; 
                        else continue; 
                    }
                    // Aceptamos Exitoso! Y lo metemos al calabozo ASYNC.
                    asignar_super_velocidad_nonblocking(infiltrado);
                    
                    struct epoll_event event_infil;
                    event_infil.data.fd = infiltrado;
                    event_infil.events = EPOLLIN | EPOLLET; 
                    epoll_ctl(cerebro_epoll, EPOLL_CTL_ADD, infiltrado, &event_infil);
                    
                    printf(" [@] [DB TCP] Client ID:%d Autenticado Exitoso a Session!.\n", infiltrado);
                }
            } 
            
            // b) SE MoviÓ Un File_Descriptor ESCLAVO. ¡Me están escribiendo un CMD!.
            else {
                 int emisor_fd = campanas_os[i].data.fd;
                 char string_tcp_client[1024];
                 int abort_tcp_c = 0;

                 // Devorar Todo Buffer TCP Hasta Secar EAGAIN (Mandatorio de EPOLLET C)
                 while (1) {
                     ssize_t leido = read(emisor_fd, string_tcp_client, sizeof(string_tcp_client) - 1);
                     
                     if (leido == -1) {
                         if (errno != EAGAIN) { abort_tcp_c = 1; }  // Crash NetWork Router
                         break;
                     } 
                     else if (leido == 0) {  
                         abort_tcp_c = 1; // EOF ! Menu C Disconnected Telnet!. Cerrar Conex.
                         break;
                     } 
                     
                     // 1. Convertir Buffers Seguros Mágicos \0 OS 
                     string_tcp_client[leido] = '\0';
                     
                     // 2. MAGIA: Ejecutar Logica Comercial Y Base De Datos Parseos.
                     handler_parseo_db(emisor_fd, string_tcp_client);
                 }

                 if (abort_tcp_c) {
                      printf("   [!] [DB TCP] Desconección de Router. DB Limpia cerrando TCP FD #%d\n", emisor_fd);
                      close(emisor_fd);
                 }
            } 
        } 
    } 

    // Destructor Global App Server C
    close(tcp_master);
    return EXIT_SUCCESS;
}
