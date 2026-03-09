/*
 * Ejercicio 2.3 — Permisos, chmod, umask y ACLs (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Entender cómo interactúan la umask del shell padre contra las peticiones de
 * un programa hijo. `umask(0)` es una técnica común en demonios (daemons) 
 * escritos en C para garantizar que sus archivos (ej. logs confidenciales en 0600) 
 * nazcan con los permisos exactos e incorruptibles.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h> // umask, chmod, stat
#include <fcntl.h>    // open, macros O_*
#include <unistd.h>   // close
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <archivo> <permisos_octales>\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s secreto.txt 0600\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filepath = argv[1];
    const char *octal_str = argv[2];

    // 1. Conversión a Entero bajo Base 8 (Octal)
    // strtol(string, char_fin_referencia, base)
    mode_t mode = (mode_t)strtol(octal_str, NULL, 8);
    
    // 2. MANEJO DE LA UMASK
    // umask() setea nuestra nueva máscara y devuelve la que estaba antes.
    // Al pedir '0', decimos "Déjame crear archivos con los permisos EXACTOS que pido".
    mode_t old_umask = umask(0);
    printf("Umask antigua interceptada: 0%03o\n", old_umask);
    
    // 3. INTENTO DE CREACIÓN 
    // O_RDWR   = Leer y Escribir
    // O_CREAT  = Crea si no existe
    // O_TRUNC  = Si existe, bórralo limpiamente a 0 bytes
    // Explicación de permisos exactos: 0%03o formatea el número como octal de 3 o más digitos.
    printf("Intentando forzar creacion con permisos exactos: 0%03o\n", mode);
    
    int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, mode);
    if (fd < 0) {
        perror("Fallo en open/create");
        // Restaurar umask antes de morir de todas formas (aunque al morir se restaura sola, es buena práctica)
        umask(old_umask);
        return EXIT_FAILURE;
    }
    
    close(fd); // Creado exitosamente, no necesitamos escribir nada, solo evaluar sus metapropiedades.

    // 4. RESTAURACIÓN DE ESTADO
    umask(old_umask);
    printf("Umask restaurada a la original de la bash.\n\n");

    // 5. DEMOSTRACIÓN EXTRA: Syscall chmod purista explícito si el archivo 
    // de hecho ya existía de otro lado y open lo que hizo fue solo sobreescribir bytes, 
    // en cuyo caso no habria podido cambiar sus metadatos base. `chmod` los aplasta sí o sí.
    if (chmod(filepath, mode) < 0) {
        perror("Aviso: chmod posterior falló");
    }

    // 6. CHEQUEO CON ACLs Vía System.
    // getfacl es parte del paquete 'acl' de Linux. Muestra el Access Control List.
    // El sistema POSIX moderno usa llamadas oscuras setxattr, acá usamos el binario para ilustrarlo.
    printf("=== Análisis getfacl del Kernel POSIX ===\n");
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "getfacl %s 2>/dev/null || stat -c '%%A %%n' %s", filepath, filepath);
    int ret = system(cmd);
    
    // Si la llamada retornó código 0, significó ejecución limpia
    if (ret == 0) {
        printf("\n✅ Permisos establecidos exitosamente.\n");
    }

    return EXIT_SUCCESS;
}
