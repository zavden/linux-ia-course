/*
 * Ejercicio 2.1 — Syscalls I/O: open/read/write/close (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Control absoluto del flujo de I/O directo con el Kernel de Linux.
 * Entender por qué en C crudo es crítico validar CADA retorno de las
 * syscalls. Un write() teóricamente podría escribir menos bytes de los 
 * que le pides (ej. si el disco se llena a la mitad de la operación), 
 * o un read() podría ser interrumpido por una señal.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>    // O_RDONLY, O_WRONLY, O_CREAT, open()
#include <unistd.h>   // read(), write(), close(), ssize_t
#include <errno.h>

/*
 * Tamaño del buffer. 
 * ¡Intenta cambiar esto a 1 (un byte) y mira cómo tu CPU llora
 * de la sobrecarga de hacer millones de context-switches al Kernel!
 * 4096 es ideal porque empata con el tamaño de caché/página del SO.
 */
#define DEFAULT_BUFFER_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <origen> <destino>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *src_path = argv[1];
    const char *dst_path = argv[2];

    int fd_in = -1;
    int fd_out = -1;
    int exit_status = EXIT_FAILURE; // Pesimismo defensivo

    // O_RDONLY = Read Only
    fd_in = open(src_path, O_RDONLY);
    if (fd_in < 0) {
        perror("Error abriendo origen");
        goto cleanup;
    }

    // O_WRONLY = Write Only
    // O_CREAT = Crear si no existe
    // O_TRUNC = Truncar a 0 bytes si ya existe (sobreescribir limpio)
    // 0644    = Permisos (-rw-r--r--) aplicados contra la umask del shell.
    fd_out = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0) {
        perror("Error abriendo/creando destino");
        goto cleanup;
    }

    // Alojamos en el Stack (la pila de memoria local). Es muy rápido.
    unsigned char buffer[DEFAULT_BUFFER_SIZE];
    ssize_t bytes_leidos;

    // Bucle clásico de lectura/escritura UNIX
    // read() devuelve cantidad de bytes que logró ller. 0 = FIN absoluto de archivo. 
    while ((bytes_leidos = read(fd_in, buffer, sizeof(buffer))) > 0) {
        
        // Cuidado: write() podría no escribir todo el bloque si hay interrupciones.
        // Implementamos un mini-bucle para forzar escribir todos los bytes extraídos.
        ssize_t bytes_escritos_totales = 0;
        
        while (bytes_escritos_totales < bytes_leidos) {
            ssize_t escritos_ahora = write(fd_out, 
                                           buffer + bytes_escritos_totales, 
                                           bytes_leidos - bytes_escritos_totales);
            
            if (escritos_ahora < 0) {
                // Si es un error real
                perror("Error FATAL escribiendo en destino");
                goto cleanup;
            }
            
            bytes_escritos_totales += escritos_ahora;
        }
    }

    if (bytes_leidos < 0) {
        // Salimos del while por error, no por EOF
        perror("Error leyendo de origen");
        goto cleanup;
    }

    exit_status = EXIT_SUCCESS; // Llegamos al fina, marcamos triunfo.

cleanup:
    // Cierre idempotente y limpieza
    if (fd_in >= 0) {
        close(fd_in);
    }
    
    // Al cerrar el archivo de salida cercioramos que el FS haga el flush real
    if (fd_out >= 0) {
        if (close(fd_out) < 0) {
            perror("Error cerrando destino (El flush pudo fallar escribiendo a disco)");
            exit_status = EXIT_FAILURE;
        }
    }

    return exit_status;
}
