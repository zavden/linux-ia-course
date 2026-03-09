/*
 * Proyecto 2 — minifind (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Diseñar una utilidad recursiva resistente. En C, la profundidad de recursión 
 * puede chocar con los límites del Stack. Aquí hacemos que los buffers grandes
 * (el path a construir) no sean re-alojados brutalmente, sino que usamos arrays locales.
 *
 * Además, enseña el uso avanzado de S_ISDIR y S_ISREG aislando la lógica de 
 * filtrado para no mezclarla con la de iteración de directorios.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>

/*
 * Estructura de Filtros. Punteros nulos (NULL) u opciones neutras significan
 * que el usuario no pidió filtrar por eso.
 */
typedef struct {
    const char *target_name;
    char target_type; // 'd', 'f', 'l', o 0
    long long target_size;   // -1 significa 'no importa'
} find_filters;

/*
 * checker:
 * Valida si un archivo específico cumple TODOS los filtros configurados.
 * Retorna true si es match (y debe imprimirse).
 */
bool match_filters(const char *filename, const struct stat *st, const find_filters *f) {
    if (f->target_name != NULL) {
        if (strcmp(filename, f->target_name) != 0) return false;
    }

    if (f->target_type != 0) {
        if (f->target_type == 'f' && !S_ISREG(st->st_mode)) return false;
        if (f->target_type == 'd' && !S_ISDIR(st->st_mode)) return false;
        if (f->target_type == 'l' && !S_ISLNK(st->st_mode)) return false;
        // Si pidiera un tipo no soportado por nuestra app, fallaría aquí.
    }

    if (f->target_size >= 0) {
        if ((long long)st->st_size != f->target_size) return false;
    }

    return true; // Si sobrevivió a todo, es que pasó.
}

/*
 * FUNCIÓN RECURSIVA: EL CORAZÓN DE FIND
 */
void do_find(const char *current_path, const find_filters *f) {
    DIR *dir = opendir(current_path);
    if (!dir) {
        // En find clásico, si no puedo abrir un dir por permisos, imprimo un warning en pantalla
        // pero NO mato el programa. Continúo explorando en otras ramas paralelas.
        fprintf(stderr, "minifind: no se puede leer %s\n", current_path);
        return;
    }

    struct dirent *ent;
    struct stat st;
    
    // IMPORTANTE: En C un PATH maximo standar es 4096 (PATH_MAX). 
    // Usamos esto para que nuestro concatenador no explote RAM en vano.
    char next_path[4096];

    while ((ent = readdir(dir)) != NULL) {
        // PREVENCIÓN INFERNAL: Jamás volver a subir al padre o refenciarse a sí mismo. Loop infinito mortal.
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        // Armamos el super string concatenado: /ruta/previa  + / + nombre.txt
        snprintf(next_path, sizeof(next_path), "%s/%s", current_path, ent->d_name);

        // Usamos lstat. Si resolvemos symlinks con stat puro y apuntan a un dir paterno,
        // causaríamos loops directos. Los symlinks deben tratarse como cosas de hoja final.
        if (lstat(next_path, &st) < 0) {
            continue; 
        }

        // 1. EVALUAR E IMPRIMIR SI MATCHEA LOS FILTROS
        // La validación se manda SOLO frente a ent->d_name (el nombre puro),
        // pero la impresión se hace al PATH completo de memoria temporal.
        if (match_filters(ent->d_name, &st, f)) {
            printf("%s\n", next_path);
        }

        // 2. RECURSIÓN
        // Si este elemento en sí mismo es un Directorio REAL, lanzaremos find_recursive dentro de él.
        if (S_ISDIR(st.st_mode)) {
            do_find(next_path, f);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    // Valores por defecto
    const char *base_path = ".";
    find_filters f = { NULL, 0, -1 };

    int i = 1;
    // Si el primer argumento es un path (no un flag como -name)
    if (argc > 1 && argv[1][0] != '-') {
        base_path = argv[1];
        i++;
    }

    // Mini parseador manual de bandera de 2 partes (Ej: "-type f")
    for (; i < argc; i++) {
        if (strcmp(argv[i], "-name") == 0 && i+1 < argc) {
            f.target_name = argv[++i];
        } else if (strcmp(argv[i], "-type") == 0 && i+1 < argc) {
            f.target_type = argv[++i][0];
        } else if (strcmp(argv[i], "-size") == 0 && i+1 < argc) {
            // El usuario da "400c". Quitamos la 'c' manual
            char size_str[32];
            snprintf(size_str, sizeof(size_str), "%s", argv[++i]);
            size_str[strlen(size_str)-1] = '\0'; // Asesinar la 'c'
            f.target_size = atoll(size_str);
        } else {
            fprintf(stderr, "Opción desconocida o incompleta: %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

    // El find oficial siempre valida e imprime el propio directorio base PRIMERO si no se filtró nada
    // Para simplificar, aquí saltamos a recorrer a sus hijos directamente.
    
    // Invocamos a la máquina infernal recursiva
    do_find(base_path, &f);

    return EXIT_SUCCESS;
}
