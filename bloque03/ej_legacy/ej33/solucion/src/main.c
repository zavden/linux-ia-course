/*
 * Ejercicio 3.3 — Pipes y Comunicación (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Diseñar una Inter-Process Communication (IPC) bidireccional "Ping Pong".
 * Obliga al alumno a enfrentarse visualmente a los 4 cabezales vivos de File
 * Descriptors de los 2 tubos anónimos, y cerrarlos quirúrgicamente en cruz.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main(void) {
    // pipe_p2c: Padre to Child (Bajada)
    // pipe_c2p: Child to Padre (Subida)
    int pipe_p2c[2]; 
    int pipe_c2p[2];

    // Instanciamos en la base de RAM del OS ambas tuberías (Llenará de fd_ints nuestros arreglos)
    if (pipe(pipe_p2c) == -1 || pipe(pipe_c2p) == -1) {
        perror("Catástrofe de inyección de tubería (pipe falló)");
        return EXIT_FAILURE;
    }

    // Hasta este punto exacto, el Kernel ha gastado 4 descriptores (ej: 3, 4, 5, 6).
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork falló. Sin clones hoy.");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) {
        // ============================================
        // CLON (HIJO)
        // ============================================
        
        // 1. RUTINA OBLIGATORIA DE SECCIÓN: Cierre Cruzado
        // Como receptor, NUNCA escribiré a la de 'Bajada'
        close(pipe_p2c[1]); 
        // Como emisor superior, NUNCA leeré a de la de 'Subida'
        close(pipe_c2p[0]); 

        char buf[BUFFER_SIZE] = {0};

        // 2. RECEPCIÓN CRÍTICA BLOQUEANTE
        // read() se "suspende/congela" solito y cede CPU a otros procesos
        // de la computadora maravillosamente hasta que alguien escriba
        // bytes en la boca 1 que quedó abierta en el Padre.
        ssize_t s = read(pipe_p2c[0], buf, sizeof(buf) - 1);
        if (s > 0) {
            printf("\t[HIJO ] => Mi padre me ha enviado orden por IPC: '%s'\n", buf);
        }

        // 3. RESPUESTA SINTÉTICA
        const char *respuesta = "Comprendido Comandante Padre. Operación efectuada desde sub-PID.";
        printf("\t[HIJO ] => Despachando retorno de estado en tubería 2...\n");
        write(pipe_c2p[1], respuesta, strlen(respuesta));

        // 4. LIMPIEZA TOTAL
        close(pipe_p2c[0]);
        close(pipe_c2p[1]);

        exit(EXIT_SUCCESS); 
    } 
    else {
        // ============================================
        // PADRE ORIGINARIO
        // ============================================
        
        // 1. CIERRE CRUZADO OPUESTO (El Padre lee Subida, y escribe en la Bajada)
        close(pipe_p2c[0]);
        close(pipe_c2p[1]);

        char mensaje_padre[] = "Ejecutar Plan Alfa Sigma Cero";
        
        // 2. TRANSMISIÓN TIERRA-LUNA
        printf("[PADRE] => Ingresando paquete codificado al conducto 1 de la RAM (%ld bytes)...\n", sizeof(mensaje_padre));
        // Un write() hacia un FD de Pipe de un OS copia instantanemente la memoria hacia un Buffer Kernell. 
        // No toca el Disco Rigido, es instantaneo y brutal en velocidad (Milisegundos nulos).
        write(pipe_p2c[1], mensaje_padre, sizeof(mensaje_padre));

        // 3. ESCUCHANDO RESPUESTA DEL ABISMO Y CONGELANDONOS ESPERANDO
        char buf2[BUFFER_SIZE] = {0};
        read(pipe_c2p[0], buf2, sizeof(buf2) - 1);

        printf("[PADRE] => Frecuencia Recibida Del Hijo Rescatada: '%s'\n", buf2);

        // 4. CIERRE FORMAL
        // Un pipe es igual que un Archivo. Si olvidas liberarlos, goteas (leak) fd's. Linux solo da 1024 fd maximos al usuario por defecto.
        close(pipe_p2c[1]);
        close(pipe_c2p[0]);

        // Sepultando su estado zombie.
        waitpid(pid, NULL, 0); 
        printf("[PADRE] => Trance terminado IPC Ping_Pong desconectado\n");
    }

    return EXIT_SUCCESS;
}
