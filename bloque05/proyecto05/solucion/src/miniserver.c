/*
 * Proyecto 5 — miniserver (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Diseñar de raíz NGINX en Base OS Networking nativos sobre un
 * ThreadPool Productor/Consumidor. El main cede todo el I/O File Systems 
 * pesados a 4 hilos pre-forkeados aislados eliminando embudos Multi-Request OOM (1M Conexiones seg). 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in 
#include <arpa/inet.h>
#include <errno.h>

/* CONSTANTES DE CONFIGURACIÓN HTTP SERVER */
#define SRV_PORT 8080
#define WORKERS_COUNT 4
#define COLA_MAXIMA 256 // Hasta 256 peticiones de espera Cíclicas

/* ========================================================
 * ARQUITECTURA DEL ESTADO COMPARTIDO (PRODUCER/CONSUMER)
 * ======================================================== */
int clients_queue[COLA_MAXIMA]; 
int cola_head = 0; // Por aquí inyecta el Padre
int cola_tail = 0; // Por aquí devora el Hijo Worker 
int cola_count = 0; 

pthread_mutex_t     q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      q_cond  = PTHREAD_COND_INITIALIZER;


/*
 * EL PARSEADOR Y RESPONDEDOR WEB DE LAS ENTRAÑAS (WORKER LOGIC)
 */
void procesar_peticion_http(int client_fd) {
    char in_buffer[1024] = {0};
    
    // Leer el string puro que manda el Navegador remotamente (Las lineas crudas de C "GET /... HTTP.. \r\n")
    ssize_t leidos = read(client_fd, in_buffer, sizeof(in_buffer) - 1);
    if (leidos <= 0) {
        close(client_fd);
        return;
    }
    // Parseo rústico. Si el request empezaba con G entonces asumimos GET magico.
    if (in_buffer[0] == 'G' && in_buffer[1] == 'E' && in_buffer[2] == 'T') {
        
        // Magia HTTP: Esta cabecera exacta con Returns Carrigae Crudos \r\n conforma el Estándard de Internet RFC Web!
        const char *head_ok = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        const char *body_html = 
            "<html><head><title>MiniServer OS</title></head>"
            "<body style='background:#111; color:#0f0; font-family:monospace; padding:50px'>"
            "<h1>[+] CONEXION POSIX EXITOSA [+]</h1><hr>"
            "<h2>Bienvenido al Servidor Nativo en C. Socket FD Procesado correctamente.</h2>"
            "<p>Powered by Pthreads, Mutexes and RAW TCP SysCalls Extracted via Port 8080 Routing System.</p>"
            "</body></html>";
        
        // Disparamos la Ráfaga C_Web Response a la Placa Base De red Hardware
        write(client_fd, head_ok, strlen(head_ok));
        write(client_fd, body_html, strlen(body_html));
        
        printf(" [?] [Worker HTTP] Request Web GET exitoso Respondido (200 OK).\n");
    } else {
        // Chromium y sus Favicons extraños o Metodos POST rebotados
        const char *head_bad = "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n\r\n";
        write(client_fd, head_bad, strlen(head_bad));
        printf(" [!] [Worker HTTP] Request extraño rebotado / No soportado (405).\n");
    }

    // CERRAMOS O LA PESTAÑA DEL NAVEGACIÓN CHROME SE QUEDARÍA GIRANDO CARGANDO AL INFINITO ESPERANDO TU BYTE!
    close(client_fd); 
}

/*
 * EL ESCLAVO DURMIENTE (THREAD POOL WORKER)
 */
void *hilo_servidor(void *arg) {
    int id = *(int*)arg;
    
    printf("     - Worker MultiThread #%d Forjado exitóso. Durmiéndo pacífico C_OS a 0%% CPU esperando en Puerto de Socket TCP!\n", id);

    while (1) {
        int target_client_fd;

        pthread_mutex_lock(&q_mutex);
        
        // Bloqueo y suspensión hasta que el Manager Principal inyecte Clientes.
        while (cola_count == 0) {
            pthread_cond_wait(&q_cond, &q_mutex);
        }

        // --- ZONA CRITICA AISLADA (Extraigo mi cliente C_OS) ---
        target_client_fd = clients_queue[cola_tail];
        cola_tail = (cola_tail + 1) % COLA_MAXIMA; // Arrays Universales Circulares
        cola_count--;
        
        // YA OBTUVIMOS AL CLIENTE. PODEMOS SOLTAR EL MUTEX Y CORTAR RAPIDISIMO PARA QUE OTRO COMPAÑERO WORKER ATIENDA A OTRO!
        // No dejes que la Función LENTA procesar_peticion trabe el OS C mutex adentro o todos pararían la Arquitectura C++.
        pthread_mutex_unlock(&q_mutex);
        
        procesar_peticion_http(target_client_fd);
    }
    
    return NULL;
}

int main(void) {
    printf("=======================================================================\n");
    printf("============        MINISERVER T-POOL EN C PURE SOCKETS    ============\n");
    printf("=======================================================================\n\n");

    // ===================================
    // 1. INICIAR PISCINAS Y COND VARIABLES
    // ===================================
    pthread_t pool_workers[WORKERS_COUNT];
    int id_wokers[WORKERS_COUNT];
    
    for (int i = 0; i < WORKERS_COUNT; i++) {
        id_wokers[i] = i+1;
        pthread_create(&pool_workers[i], NULL, hilo_servidor, &id_wokers[i]);
    }

    // ===================================
    // 2. CONFIGURACIÓN ENRUTADOR RED C LOCAL
    // ===================================

    // SYS_CALL Socket TCP IPV4 Crudo 
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) { perror("Aviso Fallo Inyectando Antenas RAM sockets C"); return EXIT_FAILURE;}

    // Configurando opciones Universales (Importante: Prevenir el odiado error 'Address Already in Use' al reiniciar tu terminal rapido de Bash C!)
    int opt_yes = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt_yes, sizeof(opt_yes));

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0 Bind Escuchando TODAS C tus ip de red
    serv_addr.sin_port = htons(SRV_PORT);   // HostToNetworkShort() -> Magia de Little_endian Arquitectures CPU Red!.

    if (bind(server_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Aviso Binding fallido por Kernell Red Enrutada"); return EXIT_FAILURE;
    }

    if (listen(server_sock, 128) < 0) {
        perror("Aviso Fallo escuchas colas de Antenas limit OS c C"); return EXIT_FAILURE;
    }

    printf("\n  [$$] OS Kernell TCP Link Realizado. Antenas montadas. Servidor Inmortalizado C en Loop Escuchando HTTP Local Puerto 8080.\n");
    printf("  [$$] Entra desde el navegador en http://localhost:%d o apágualo de tirón con Ctrl+C.\n\n", SRV_PORT);

    // ===================================
    // 3. EL BUCLE MAESTRO GERENCIAL OS WAIT TCP INFINITO
    // ===================================
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        // Bloqueo y Pasmo C Kernell (Accept) : Congela al Papacito Main entero... hasta que un Pïng entre a la PC de Magia por IPV4.
        int cli_fd = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        
        if (cli_fd < 0) {
            perror("\nFallo misterioso C_Error al acatar request OS accept FD.");
            continue; 
        }

        /* 
         * DELEGACIÓN INSTANTÁNEA ASÍNCRONA A ESCLAVOS DORMIDOS
         * Main mete al socket nuevo (el file descriptor) en la Cola 
         * y hace repicar 1 vez la Trompeta de alerta POSIX (cond_signal) a la matriz para no colgarse a leer él mismo!
         */
        pthread_mutex_lock(&q_mutex);
        
        if (cola_count < COLA_MAXIMA) {
            clients_queue[cola_head] = cli_fd;
            cola_head = (cola_head + 1) % COLA_MAXIMA;
            cola_count++;
            
            // Toque Mágico C: Despierta a 1 SOLO Worker al azar que esté dormido (Suficiente c puestamente!).
            pthread_cond_signal(&q_cond);
        } else {
            // Cola sobresaturadísima. NGINX suele responder error 503 HTTP Server Busy At C OS limit!.
            printf("\t[!!] Alerta Max C: Servidor colpasó por Ddos Overflow en buffer de colas C. Suelte!\n");
            close(cli_fd); 
        }

        pthread_mutex_unlock(&q_mutex);
    }

    // Esta línea no será alcanzada porque tu while 1 no tiene forma programática break pacifica para fines educativos server (solo interrupciones kill -9 C Crtl+C bash C abortos)
    close(server_sock); 
    return EXIT_SUCCESS;
}
