/*
 * Ejercicio 6.3 — EPOLL (El corazón de NodeJS y NGINX) (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * La Arquitectura Suprema de Alto Rendimiento en Concurrencias. 
 * Sustituiremos Hilos por Eventos (Event_Driven Programming C Pure). 
 * Se maneja un Router_Event Loop con Notificaciones masivas O(1) directas del Linux Kernel C.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h> // DIOS MÁXIMO DE REDES C LINUX_OS

#define APP_PORT 9090
#define MAX_POLLING_EVENTS 64

/*
 * CONVIERTE DESCRIPTORES TCP A FLASH-ASYNC
 */
int asignar_super_velocidad_nonblocking(int fd_socket) {
    int blz_flags = fcntl(fd_socket, F_GETFL, 0);
    if (blz_flags == -1) return -1;
    return fcntl(fd_socket, F_SETFL, blz_flags | O_NONBLOCK);
}

int main(void) {
     printf("============== SERVIDOR EPOLL MÁXIMO RENDIMIENTO C (PORT %d) ==============\n\n", APP_PORT);

    // ===================================
    // 1. CREACIÓN DEL HOST C
    // ===================================
    int master_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if (master_tcp < 0) { perror("OS Fatal Creación de antena S"); return EXIT_FAILURE; }
    
    int tp = 1; setsockopt(master_tcp, SOL_SOCKET, SO_REUSEADDR, &tp, sizeof(tp));

    struct sockaddr_in mis_reglas;
    memset(&mis_reglas, 0, sizeof(mis_reglas));
    mis_reglas.sin_family = AF_INET;
    mis_reglas.sin_addr.s_addr = INADDR_ANY; 
    mis_reglas.sin_port = htons(APP_PORT);

    if (bind(master_tcp, (struct sockaddr *)&mis_reglas, sizeof(mis_reglas)) < 0) {
       perror("Kernell Rechazó mi Puerto"); return EXIT_FAILURE; 
    }

    // MANDATORIO: Devolver Master TCP al modo flash o el accept del loop fallará bloqueándote 
    // C toda la red frente a pings zombies que se arrepientan.
    asignar_super_velocidad_nonblocking(master_tcp);

    if (listen(master_tcp, SOMAXCONN) < 0) { 
        perror("Aviso Errado Escuchas"); return EXIT_FAILURE; 
    }

    // ===================================
    // 2. INVOCANDO LA API OBSERVADORA KERNEL
    // ===================================
    int cerebro_epoll = epoll_create1(0);
    if (cerebro_epoll == -1) {
        perror("Fallo tu RAM instanciando tu Matrix Epoll System Kernel"); return EXIT_FAILURE;
    }

    struct epoll_event vigilar_master;
    vigilar_master.data.fd = master_tcp;  
    
    // EPOLLIN (Avísame C cuando a este men se le pueda LEER (Entren peticiones entrantes o envíe chats)).
    // EPOLLET (Edge Triggered). Super avanzado C. Si lee 8 bytes y dejaste 2... epoll_wait NO volverá a despertarte por esos 2 "sobrantes".
    // EPOLLET fuerza a que limpies hasta el ultimo byte `EAGAIN` de un golpe salvando millones de ciclos CPU repitiendo Loops lentos!.
    vigilar_master.events = EPOLLIN | EPOLLET;  
    
    if (epoll_ctl(cerebro_epoll, EPOLL_CTL_ADD, master_tcp, &vigilar_master) == -1) {
         perror("No se pudo añadir Central de Mando a Matrix de Redes c"); return EXIT_FAILURE;
    }

    // Array Gigante Asíncrono para guardar las C respuestas crudas devueltas del wait().
    struct epoll_event *campanas_os = calloc(MAX_POLLING_EVENTS, sizeof(struct epoll_event));


    printf("  [+] ¡Servidor Single-Thread Indestructible Activado C+!\n");
    printf("  [+] Abre hasta 10,000 Pestañas Bash con `nc 127.0.0.1 %d` (El I/O las absorberá solitas sin hilos!).\n", APP_PORT);

    // ===================================
    // 3. EVENT LOOP NODESJS ASÍNCRONO MAESTRO 
    // ===================================
    while (1) {

        /* 
         * PAUSADO INTELIGENTE (EL MULTIPLEXOR MÁGICO) 
         * Si mandas -1 (Infinito Timeout), El C Papa Principal se DORMIRÁ a 0% CPU !
         * Pero CUALQUIERA, literalmete Cualquiera de las 64 antenas posibles adentro si se 
         * mueven un milimetro despertaran al Padre C al instante retornandole Cuantos se movieron ! 
         * [ O(1) Rendimiento Colosal OS ].
         */
        int toques_despiertos = epoll_wait(cerebro_epoll, campanas_os, MAX_POLLING_EVENTS, -1);
        
        if (toques_despiertos == -1) { 
           perror("Error Despertador epoll C. Crash Red"); break;
        }

        // --- ENRUTANDO LA RESPUESTA ---
        for (int i = 0; i < toques_despiertos; i++) {
            
            // a) Chequeos de Errores Vagos de WiFi/Ethernet  (EPOLLERR = Se rompió cable) o EPOLLHUP (Colgp de golpe C)
            if ((campanas_os[i].events & EPOLLERR) || (campanas_os[i].events & EPOLLHUP) || (!(campanas_os[i].events & EPOLLIN))) {
                  unsigned int target = campanas_os[i].data.fd;
                  fprintf (stderr, "  [!] Epoll Reporta Error Severo/Cierre Aspero de C_Socket #%d. Matándolo de radar...\n", target);
                  close (campanas_os[i].data.fd);
                  continue;
            } 
            
            // b) ¿ACASO C FUÉ EL PAPÁ EL QUE SE MOVIÓ? (Si sí: Significa un Client_New TCP Accept C)
            else if (master_tcp == campanas_os[i].data.fd) {
                
                // Bucle Interno While infinito de Accepts... PORQUE ESTAMOS EN MODO FLASH ASYNC!.
                // Podrían haber entrado 5 gentes C en el último milisegundo al Papa, asi que 
                // hacemos Accept for loop hasta que arroje Error EAGAIN y así no perdemos ni 1 men de vista!
                while (1) {
                    struct sockaddr in_addr;
                    socklen_t in_len = sizeof(in_addr);
                    int infiltrado = accept(master_tcp, &in_addr, &in_len);
                    
                    if (infiltrado == -1) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                             // "Papi, ya atrapaste a todos los de la puerta fila. Ve con Dios (Break while)".
                             break;
                        } else {
                             perror ("Accept Mágico Kernell Defectuoso C!"); break;
                        }
                    }

                    // Aceptamos Exitoso! Y lo metemos al calabozo ASYNC.
                    asignar_super_velocidad_nonblocking(infiltrado);
                    
                    struct epoll_event event_infil;
                    event_infil.data.fd = infiltrado;
                    event_infil.events = EPOLLIN | EPOLLET; // EdgeTrigger C (Súper veloz ahorrando System Calls inútiles!)
                    
                    // Inyección C Final de Radar!
                    if (epoll_ctl(cerebro_epoll, EPOLL_CTL_ADD, infiltrado, &event_infil) == -1) {
                        perror("Fracaso Metiendo al Radar C Esclavo."); break;
                    }
                    
                    printf("\n  \033[1;36m[>] Usuario C_Nativo #FD %d Conectado y En-Radar-Asincrónico!\033[0m\n", infiltrado);
                }
                continue; // Vamos a Ver el Siguiente campanazo de otro men!
            } 
            
            // c) SI NO FUE EL PAPA C,... ¡ENTONCES FUE UN ESCLAVO USUAARIO VIEJO HABLANDO UN STRING AL CHAR C CHAT!
            else {
                 int emisor_fd = campanas_os[i].data.fd;
                 char basuras[512];
                 int cerrado_muerto = 0;

                 // For loop OBLIGATORIO DE LECTURA por el bendito EPOLLET (Devorarás el socket al cien asincronamente) C.
                 while (1) {
                     ssize_t contador = read(emisor_fd, basuras, sizeof(basuras) - 1);
                     
                     if (contador == -1) {
                         if (errno != EAGAIN) {
                             perror("   [!] Leer Basuras en Client C Fracasó Maleducado"); 
                             cerrado_muerto = 1;
                         } 
                         // Si es EAGAIN... "Leíste todito lo que el men tenia que decir. Paz.". (Break pacífico general).
                         break;
                     } 
                     else if (contador == 0) {
                         // End Of File. (Se fué el men de la TCP C)
                         cerrado_muerto = 1;
                         break;
                     } 
                     
                     // Magia de Reconstrucción e Impresión C ! String.
                     basuras[contador] = '\0';
                     if (basuras[contador-1] == '\n') basuras[contador-1] = '\0';
                     printf("\t[CLIENTE %d] Escribe rápido C : %s\n", emisor_fd, basuras);
                     
                     // Eco C Response
                     write(emisor_fd, " [√] Server_Epoll ACKs (Recibido C_Asincrono)\n", 47);
                 }

                 if (cerrado_muerto) {
                      printf("   \033[1;31m[<] Usuario C_Nativo #FD %d Abortó Tímidamente de Network. ¡Eliminado OS Matrix!\033[0m\n", emisor_fd);
                      // Destrucción Formal (Cierras FD y MAGICAMENTE EL OS KERNEL LO BORRA DE LA CENTRAL MATRIX EPOLL C MUNDIAL SOLO!! Sin EPOLL_CTL DEL C!)
                      close(emisor_fd);
                 }
            } // Termina Lógica Mensajería C Client
        } // Termina Loop Interno Acatamientos
    } // Vuelve Arriba el Loop Padre C

    free(campanas_os);
    close(master_tcp);

    return EXIT_SUCCESS;
}
