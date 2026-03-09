/*
 * Ejercicio 4.3 — Memoria Compartida POSIX (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Combinar todos los conceptos de OS C en un IPC Atómico de alta velocidad.
 * Muestra el peligro inherente de requerir Sincronización (Aquí usamos sleep rústico) 
 * y la crucialidad absoluta del Cleanup de los descriptores shm, los cuales 
 * el Kernel es renuente a limpiar al cerrar el programa espontaneamente.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>   // mmap, shm_open, shm_unlink
#include <sys/wait.h>
#include <sys/stat.h>   // mode_t
#include <errno.h>

/*
 * Las rutas de Inter-Process COM(IPC) a nivel POSIX estricto OBLIGAN a que el
 * namespace identificador comience con un (1) solo y único Slahs '/'.
 * De lo contrario dará "Invalid Argument" asquerosamente inentendible.
 */
#define SHM_PATH "/canal_telepatico_43"

/* 
 * Una PÁGINA típica de Arquitectura x86 o ARM suele ser 4096 bytes. (4KB).
 * Si truncas esto a "3 bytes", el SO te la hincha dándote y bloqueando RAM de 4096 KB en físico
 * independientemente a tu limitante superficial. Es la talla más eficiente.
 */
#define SZ_TALLA_VIRTUAL 4096

int main(void) {
    // ============================================
    // 1. ANIDANDO EL OBJETO GLOBAL FANTASMA
    // ============================================
    
    // shm_open retorna un File Descriptor ordinario (int). Pide los mismos Flags que `open`.
    int fd_shm = shm_open(SHM_PATH, O_CREAT | O_RDWR, 0666);
    if (fd_shm == -1) {
        perror("Error crítico solicitando segmento crudo del MMU en el Kernel.");
        return EXIT_FAILURE;
    }

    // 2. INFLADO EXPANCSIVO AL ESPACIO DEMANDADO
    // Nuevo de agencia: El objeto C fue devuelto con un tamaño ridículo de 0.
    // Ojo: Si ya existiera de una ejecución ayer en la pc, ftruncate NO truncará los viejos datos
    // a borrones si lo pones a un size igual al que tenía (se quedaria igual con basura anterior, o le mocharía las partes sobrantes si achicas).
    if (ftruncate(fd_shm, SZ_TALLA_VIRTUAL) == -1) {
        perror("Error de inflación ftruncate asignado RAM dimensional.");
        close(fd_shm);
        shm_unlink(SHM_PATH); // Limpiamos basura huérfana
        return EXIT_FAILURE;
    }

    // 3. ENCADENAMIENTO DEL PUNTERO RAM (Memoria Privada VS Pública MMAP)
    // El Santo grial es MAP_SHARED. Los procesos Foráneos futuros C podrán tocarla también.
    char *vinc_data = mmap(NULL, SZ_TALLA_VIRTUAL, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
    if (vinc_data == MAP_FAILED) {
        perror("Colapso del Puntero MMAP interconectando Shared Objects.");
        close(fd_shm);
        shm_unlink(SHM_PATH);
        return EXIT_FAILURE;
    }

    /* 4. OPTIMIZACIÓN PURIFICADORA DEL OS
     * El File Descriptor (número `fd_shm`) sólo valía para negociar con el kernel el
     * encadenamiento arriba. A partir de AQUÍ MISMO, ya es obsoleto y cerramos la conexión FD clásica.
     * Toda la transofrmacion se hara atacando de lleno violentamente a "vinc_data[X]"!
     */
    close(fd_shm);

    // ============================================
    // SEPARACION DE CLONES MULTI-HILOS (FORK)
    // ============================================
    pid_t cloner = fork();
    if (cloner < 0) {
        perror("Aborto multi-c.");
        // Aseo forzoso
        munmap(vinc_data, SZ_TALLA_VIRTUAL);
        shm_unlink(SHM_PATH);
        return EXIT_FAILURE;
    }
    
    if (cloner == 0) {
        // --- SECUELA HIJO ---
        
        // 5. ESPERA RÚSTICA SIN CONDICIONANTES (No usar IPC Semáforos aun)
        // El hijo debe esperar que el padre siembre su huella dactilar estelar.
        sleep(1); 
        
        // El hijo despiera, mira, y en C extrae las variables del Abismo Vacio de la SharedMem
        printf("\n    [HIJO ] => Receptáculo Atómico Leído Exitosamente: -> [ %s ]", vinc_data);

        // 6. RESPONDIENDO DE NUEVO ESCRIBIENDO A LA MISMA MEMORIA GLOBAL COMPARTIDA
        // No hay System calls pesadas como WRITE() aquí... !
        strcpy(vinc_data, "Reporte Final Entregado Papi: Mision C cumplida. Cierres Inminentes.");
        
        // munmap opcional para formalismo (al morir con exit el kernel Linux lo desaloja igual).
        munmap(vinc_data, SZ_TALLA_VIRTUAL);
        exit(EXIT_SUCCESS);
        
    } 
    else {
        // --- SECUELA PADRE PRINCIPAL ---

        // 5. SIEMBRA O ESTRUCTURACION Y ENCRIPTACION A STRING
        // Un simple "Strcpy" normal C va a alterar magicamente las paginas Hardware compartidas del OS.
        const char *m = "Secreto Nacional De Inteligencia IPC: Claves Válidas Listas!";
        printf("\n[PADRE] => Ingresando paquete codificado al Charco Local (Puntero C) (%ld bytes)...", strlen(m));
        strcpy(vinc_data, m);

        // 6. BLOQUEO ESPERA REAPER ZOMBIE
        waitpid(cloner, NULL, 0);

        // 7. VERIFICANDO POSESION/MUTACION HIJA TRAS TELEPATÍA
        printf("\n[PADRE] => Volviendo de la muerte, hijo terminó.\n");
        printf("[PADRE] => Frecuencia Cambiada Vía Mente (Lectura en charco local): -> [ %s ]\n", vinc_data);

        // ============================================
        // 8. PURGA OBLIGATORIA DEL DEMONIO SHM
        // ============================================
        if (munmap(vinc_data, SZ_TALLA_VIRTUAL) != 0) {
            perror("Aviso menor: No pude desenlazar el cache puntero Mmap Virtual");
        }
        
        // EXTREMO CUIDADO! A diferencia de `malloc()`, la RAM Compartida *SOBREVIVIRÁ* a que tu progama muera
        // Cuelgue o termines con Exit. Un `Crtl+C` dejurará los Megabytes ocupados para simpre en las sombras del SO.
        // SHM_UNLINK rompe el vinculo desde el registro gubernamental OS Posix formalmente regresandole su valiosa RAM C++ general
        if (shm_unlink(SHM_PATH) != 0) {
            perror("¡Aviso! Falla catastrófica al Desmontar RAM fantasma del IPC... El charco /dev/shm_path se quedò sucio devorando disco!");
        }

        printf("\n>>> Desconexiones Exitosas. Apagado General Pacifico 200... \n");
    }

    return EXIT_SUCCESS;
}
