/*
 * Ejercicio 4.2 — Memoria Mapeada (mmap) (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Control de mapeo global usando RAM.
 * Muestra las syscalls de fstat para cálculo estricto de limites y 
 * la validación de un Page de mmap mediante MS_SYNC (Syncing real al Disco físico).
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h> // mmap, munmap, msync, PROT_*, MAP_*
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h> // fstat
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo_existente_a_mutar>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filepath = argv[1];

    // 1. OBTENIENDO LA ESTACIÓN DE CANAL (FILE DESCRIPTOR LOCAL)
    // O_RDWR: Necesitamos ambas direcciones de lectura si queremos PROT_READ | PROT_WRITE.
    int fd = open(filepath, O_RDWR);
    if (fd < 0) {
        perror("Error critico destapando y abriendo archivo");
        return EXIT_FAILURE;
    }

    // 2. TALLAJE ABSOLUTO (El mmap explota infalible si tratas de mapear tamaño nulo 0 bytes)
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("Falla sondeando peso por File Descriptor en fstat");
        close(fd);
        return EXIT_FAILURE;
    }

    // Si el usuario nos mandase un touch archivo vacio "0 bytes", abortamos sano para 
    // no lanzar segfaults incontrolables desde mmap
    if (st.st_size == 0) {
        fprintf(stderr, "Aviso Kernell: No me jodas, no puedes mapear en RAM un archivo que mide 0 dimension bytes.\n");
        close(fd);
        return EXIT_FAILURE;
    }

    // 3. LA GRAN INVOCACIÓN MÁGICA DE POSIX PAGE FAULT (MMAP)
    // PROT = Protections de seguridad que la Virtual Memory del CPU usará verificando nuestro apuntador.
    // MAP_SHARED = Es un milagro que hace que si tocamos, Linux ensucie sus "Page Cache" y las envíe al Disco real. 
    // MAP_PRIVATE = Habría hecho que si toco el indice, ¡se clone sigilosamente dejándome mi mutación en RAM exclusiva mía pero jamás tocando de verdad al disco final! (Copy/On/Write).
    char *map_data = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    /* 
     * EL RETORNO MALDITO (-1 MÁGICO) 
     * Mmap no dueleve simple "NULL". Devuelve un objeto crudo cast a pointer "MAP_FAILED" 
     * y es mandatorio compararlo de esta especifica y sucia forma
     */
    if (map_data == MAP_FAILED) {
        perror("Fallo atómico interconectando Ram con Mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("1) Extensión Mapeada en la Estructura RAM C exitosamente (%ld Bytes).\n", (long)st.st_size);
    printf("2) Extrayendo mágicamente el primer byte original antes del hackeo... [ '%c' ]\n", map_data[0]);

    // 4. EL ATAQUE INVISIBLE A RAM (QUE MUTARÁ REPERCUTIENDO DISCOS)
    printf("3) Mutando el char base subyacente del byte 0 asignándole una 'A'...\n");
    map_data[0] = 'A';

    // 5. SINCRONIZANDO (FLUSHING/FSYNC FORZOSO A PLATOS DE DISCO)
    // "MS_SYNC" le dice que no retorne esta función C hasta no saber con el alma de 
    // que el FS y Kernel guardaron tu 'A' sin riesgo a apagon de corrientes luz ni cortes.
    if (msync(map_data, st.st_size, MS_SYNC) < 0) {
        perror("Aviso: Fallo un msync flushing a disco");
    } else {
        printf("4) Flush Msync Garantizado Limpio.\n");
    }

    // 6. DESTRUYENDO PUENTES MÁGICOS Y ARRENDAMIENTOS
    // munmap exige que le dictes la misma talla exacta para borrar los segmentos de memoria global correctos!
    if (munmap(map_data, st.st_size) < 0) {
        perror("Aviso de Error Munmap fallando limpieza");
    }

    // Ya desenlazados, este cierre de FD no afecta ni quita RAM porque la ram fué matada un bloque arriba.
    close(fd);
    
    printf("5) Apagón Limpio Síncrono Total. ¡Hemos mutado el disco sin re-read ni re-write!\n");

    return EXIT_SUCCESS;
}
