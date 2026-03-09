/*
 * Ejercicio 2.4 — Directorios: opendir, readdir, stat (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Combinar el stream asíncrono dirent (Directory Entry) devuelto por el kernel,
 * con la consulta masiva de metadatos pesados devuelta por lstat.
 * Introduce macros fundamentales como S_ISDIR y S_ISREG para desempacar la 
 * máscara binaria `st_mode` que devuelve lstat.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h> // open/read/close dir
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <directorio>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *dir_path = argv[1];

    // 1. ABRIR EL STREAM DEL DIRECTORIO
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Fallo opendir");
        return EXIT_FAILURE;
    }

    struct dirent *ent;
    struct stat st;

    printf("%-10s %-5s %s\n", "Size(B)", "Type", "Name");
    printf("------------------------------------------\n");

    // 2. RECUPERACIÓN ENTIDAD-POR-ENTIDAD
    // readdir nos va entregando archivos uno a uno hasta que no quedan más (NULL)
    while ((ent = readdir(dir)) != NULL) {
        
        // Ignoramos los enlaces especiales actuales y padres para no infinitizar recursiones futuras.
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        // 3. RECONSTRUCCIÓN DE RUTA ABSOLUTA/RELATIVA
        // readdir SOLO nos dio "passwd". Nosotros debsmos armar "/etc/passwd".
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, ent->d_name);

        // 4. SONDA DE METADATOS (lstat)
        // Usamos lstat() en vez de stat() porque si un archivo es un Sysmlink, 
        // queremos detalles del Symlink mismo, no del archivo al que apunta.
        if (lstat(full_path, &st) < 0) {
            perror("Aviso parcial: Fallo lstat en un archivo del directorio");
            // No matamos la ejecución, simplemente skipeamos este archivo (quizás era root o despareció milidsegundos atrás).
            continue;
        }

        // 5. PARSEO DEL METADATO MÁGICO `st_mode`
        // st_mode es un entero de 16 bits. Los bits altos dictan qué es.
        // POSIX nos regala macros que toman un AND binario y responden booleanos.
        char type_str = '?';
        if (S_ISREG(st.st_mode)) {
            type_str = '-'; // File regular
        } else if (S_ISDIR(st.st_mode)) {
            type_str = 'd'; // Directorio
        } else if (S_ISLNK(st.st_mode)) {
            type_str = 'l'; // Enlace simbólico (symlink)
        } else if (S_ISCHR(st.st_mode)) {
            type_str = 'c'; // Character device (/dev/tty)
        } else if (S_ISBLK(st.st_mode)) {
            type_str = 'b'; // Block device (/dev/sda)
        }

        // 6. IMPRESIÓN FORMATEADA
        // %-10lu significa "Long Unsigned int, ocupando obligatoriamente 10 espacios y alineado al guión (izquierda)"
        printf("%-10lu %-5c %s\n", (unsigned long)st.st_size, type_str, ent->d_name);
    }

    // 7. CIERRE Y LIBERACIÓN DE RECURSOS DEL SISTEMA OPEATIVO
    closedir(dir);

    return EXIT_SUCCESS;
}
