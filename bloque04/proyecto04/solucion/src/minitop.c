/*
 * Proyecto 4 — minitop (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Entender que en Linux "Todo es un archivo". `/proc` no existe en el disco duro,
 * es una base de datos de RAM disfrazada de carpeta y archivos TXT arrojados por
 * el Kernel mismo que podemos parsear de la nada infinita.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>

#define MAX_LINE 256

/* Limpieza de Pantalla Terminal Emulando Clear/Top Clásico */
void ansi_clear_screen() {
    printf("\033[2J\033[H");
}

/* 
 * PARSEADOR DEL CEREBELO GLOBAL: RAM Y CARGA
 */
void print_global_stats() {
    FILE *f_mem = fopen("/proc/meminfo", "r");
    if (!f_mem) return;

    long memTotal = 0, memFree = 0, buffers = 0, cached = 0;
    char line[MAX_LINE];
    
    // Leemos el tesoro de RAM línea por línea hasta sacar la data mágica
    while (fgets(line, sizeof(line), f_mem)) {
        if (strncmp(line, "MemTotal:", 9) == 0) sscanf(line, "MemTotal: %ld kB", &memTotal);
        if (strncmp(line, "MemFree:", 8) == 0) sscanf(line, "MemFree: %ld kB", &memFree);
        if (strncmp(line, "Buffers:", 8) == 0) sscanf(line, "Buffers: %ld kB", &buffers);
        if (strncmp(line, "Cached:", 7) == 0) sscanf(line, "Cached: %ld kB", &cached);
    }
    fclose(f_mem);

    long used_aprox = memTotal - memFree - buffers - cached;
    double porcentaje = 0.0;
    if (memTotal > 0) porcentaje = ((double)used_aprox / memTotal) * 100.0;

    FILE *f_load = fopen("/proc/loadavg", "r");
    char load_str[64] = "N/A";
    if (f_load) {
        fgets(load_str, sizeof(load_str), f_load);
        // Quitamos enter si lo traía
        size_t len = strlen(load_str);
        if (len > 0 && load_str[len-1] == '\n') load_str[len-1] = '\0';
        fclose(f_load);
    }

    printf("\n\033[1;36m===== MINITOP - LINUX PROCESS KERNEL VIEW =====\033[0m\n");
    printf("\033[1;33mRAM Arox:\033[0m %ld MB / %ld MB (\033[1;31m%.1f%%\033[0m Utilizada | %ld MB Libres y en Cache VFR)\n", 
           used_aprox / 1024, memTotal / 1024, porcentaje, (memFree + cached) / 1024);
    printf("\033[1;33mLoad Avg:\033[0m [ %s ] \n\n", load_str);
}


/* 
 * DETECTA CARPETAS PID IGNORANDO CONFUSIONES (/proc/cpuinfo, /proc/sys, etc no nos interesan)
 */
int is_numeric_dir(const char *name) {
    for (int i = 0; name[i] != '\0'; i++) {
        if (!isdigit((unsigned char)name[i])) {
            return 0; // Contiene basura/letras formales
        }
    }
    return 1;
}

/* 
 * ESPIO INDIVIDUAL: AUTOBSIA A UN PROCESO DEL OS (Parseando de 2 fuentes distintas)
 */
void process_pid(const char *pid) {
    char path_stat[256];
    char path_status[256];
    
    // '/proc/<PID>/stat' : Archivo que junta nombre, Estado, Prioridad entre otros en 50 columnas puras C Csv-like.
    snprintf(path_stat, sizeof(path_stat), "/proc/%s/stat", pid);
    snprintf(path_status, sizeof(path_status), "/proc/%s/status", pid);

    FILE *fp_stat = fopen(path_stat, "r");
    if (!fp_stat) return; // Ha muerto el proceso este mismo microsegundo! Magia de Concurrencia (Archivos volatiles). Ignoramos pacífico

    int _p; // dummy temporal throwaway var
    char comm_name[256] = ""; // Nombre "exe" (va envuelto en '(bash)')
    char state = '?';
    
    // Leemos rudamente las tres primeras columnas C ("1425 (bash) S")
    if (fscanf(fp_stat, "%d (%255[^)]) %c", &_p, comm_name, &state) != 3) {
        fclose(fp_stat);
        return;
    }
    fclose(fp_stat);

    // Cruzando Data con Status para RSS de Resident Mem (MegaBYTES)
    // (Porque en stat viene cifrado en Paginas y es horrible de multiplicar rapido para el OS vs Status que da MegaBites Limpios KB).
    FILE *fp_status = fopen(path_status, "r");
    long vm_rss = 0;
    if (fp_status) {
        char line[MAX_LINE];
        while (fgets(line, sizeof(line), fp_status)) {
            // "VmRSS:     10200 kB"
            if (strncmp(line, "VmRSS:", 6) == 0) {
                sscanf(line, "VmRSS: %ld kB", &vm_rss);
                break;
            }
        }
        fclose(fp_status);
    }
    
    // Solo me molesto en imprimir procesos que importan y chupan RAM hoy (evitamos kernel threads durmiendo por defecto)
    if (vm_rss > 100 || state == 'R' || state == 'Z') {
         printf("\033[1;32m%-8s\033[0m | %-2c | %-8ld MB | %-20s\n", pid, state, vm_rss / 1024, comm_name);
    }
}

int main(void) {
    // Demo Loop de Refresh (En un server real, Top repite todo N Vueltas). 
    // Para no bloquear la automatizacion y el QA Tester, limitaremos a 2 vueltas demo por seguridad en Github Acions/etc.
    int vueltas_tester = 2;

    while (vueltas_tester > 0) {
        ansi_clear_screen();
        print_global_stats();
        
        printf("\033[1;37m%-8s | %-2s | %-11s | %-20s\033[0m\n", "PID", "ST", "RES. RAM", "COMMAND/ALIAS");
        printf("---------+----+-------------+---------------------\n");

        // 1. ABRIR EL ABISMO SAGRADO
        DIR *dir = opendir("/proc");
        if (!dir) {
            perror("Error colapso kernell. Tu Sistema no es un Linux o esta encadenado brutalmente tu Docker.");
            return EXIT_FAILURE;
        }

        struct dirent *ent;
        
        // 2. ITERANDO FANTASMAS UNO POR UNO
        while ((ent = readdir(dir)) != NULL) {
            // Ignorar '.' y '..' y otras mulas
            if (ent->d_name[0] == '.') continue;
            
            // Si es puro número, es una bestia del OS llamada PID.
            if (is_numeric_dir(ent->d_name)) {
                process_pid(ent->d_name);
            }
        }
        closedir(dir);

        vueltas_tester--;
        if (vueltas_tester > 0) {
           printf("\n  >> Refreshing en 2 segundos C_OS (Vueltas Faltantes QA Demo: %d)...\n", vueltas_tester);
           sleep(2);
        }
    }

    printf("\n  >>> MINITOP FINALIZADO.\n");
    return EXIT_SUCCESS;
}
