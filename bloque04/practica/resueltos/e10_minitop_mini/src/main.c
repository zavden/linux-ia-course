#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_numeric_name(const char *s) {
    /* En /proc, los procesos viven en directorios cuyo nombre es el PID. */
    if (*s == '\0') return 0;
    for (size_t i = 0; s[i] != '\0'; ++i) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

static int parse_meminfo_file(const char *path, long *total_kb, long *free_kb, long *buffers_kb, long *cached_kb) {
    /* Reutilizamos la misma idea del parser del ejercicio anterior. */
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char line[256];
    *total_kb = *free_kb = *buffers_kb = *cached_kb = -1;

    while (fgets(line, sizeof(line), f) != NULL) {
        /* Parseo tolerante al orden de líneas. */
        if (sscanf(line, "MemTotal: %ld kB", total_kb) == 1) continue;
        if (sscanf(line, "MemFree: %ld kB", free_kb) == 1) continue;
        if (sscanf(line, "Buffers: %ld kB", buffers_kb) == 1) continue;
        if (sscanf(line, "Cached: %ld kB", cached_kb) == 1) continue;
    }

    fclose(f);
    return (*total_kb >= 0 && *free_kb >= 0 && *buffers_kb >= 0 && *cached_kb >= 0) ? 0 : -1;
}

/*
 * Lee status simplificado de un PID ficticio/real:
 * espera líneas tipo "Name:" y "VmRSS:".
 */
static void print_pid_status(const char *proc_root, const char *pid) {
    char path[512];
    /* Ruta típica: <proc_root>/<pid>/status */
    snprintf(path, sizeof(path), "%s/%s/status", proc_root, pid);

    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[256];
    char name[128] = "?";
    long rss_kb = -1;

    while (fgets(line, sizeof(line), f) != NULL) {
        /* Nos quedamos con nombre del proceso y su RSS en kB. */
        if (sscanf(line, "Name: %127s", name) == 1) continue;
        if (sscanf(line, "VmRSS: %ld kB", &rss_kb) == 1) continue;
    }

    fclose(f);
    printf("pid=%s name=%s rss_kb=%ld\n", pid, name, rss_kb);
}

int main(int argc, char **argv) {
    /* Entradas explícitas para facilitar tests con fixtures locales. */
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <meminfo_path> <proc_root>\n", argv[0]);
        return EXIT_FAILURE;
    }

    long total, free_kb, buffers, cached;
    if (parse_meminfo_file(argv[1], &total, &free_kb, &buffers, &cached) == -1) {
        fprintf(stderr, "No se pudo parsear meminfo\n");
        return EXIT_FAILURE;
    }

    /* Métrica global simplificada de memoria usada. */
    long used = total - free_kb - buffers - cached;
    printf("mem_used_kb=%ld\n", used);

    /* Recorremos el pseudo-filesystem de procesos. */
    DIR *d = opendir(argv[2]);
    if (!d) {
        perror("opendir");
        return EXIT_FAILURE;
    }

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        /* Ignoramos todo lo que no sea un PID numérico. */
        if (!is_numeric_name(ent->d_name)) continue;
        /* Emitimos fila resumida de cada proceso encontrado. */
        print_pid_status(argv[2], ent->d_name);
    }

    closedir(d);
    return EXIT_SUCCESS;
}
