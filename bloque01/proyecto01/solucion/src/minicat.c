/*
 * Proyecto 1 — minicat (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Recrear `cat` combinando stdio.h para lectura en bloque, parseo de args via getopt_long, 
 * y gestión de entradas estándar condicionales.
 * Enfatiza fuertemente el chequeo de retornos (return checks) porque cat lee archivos que 
 * a menudo el sistema te denegará por falta de permisos o existencia.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>

static bool opt_number = false;
static bool opt_number_nonblank = false;
static int line_counter = 1; // Contador global perpetuo (no resetea por archivo)

/*
 * FILE *in representa el "stream" base de donde leer (un archivo o tu teclado).
 * Leerá un caracter a la vez buscando \n para poder insertar "  1 " al principio de la línea.
 */
void cat_file(FILE *in, const char *filename, int *exit_status) {
    if (!in) {
        // perror pegará nuestro string "minicat: archivo.txt" con el error real
        // por ej: "minicat: archivo.txt: Permission denied". (Hermosa convención de bash).
        fprintf(stderr, "minicat: ");
        perror(filename);
        *exit_status = EXIT_FAILURE; // Acumulamos el error pero no matamos programa aún
        return;
    }

    int c;
    bool at_line_start = true;
    bool previous_was_blank = false;

    // fgetc extrae 1 byte a la vez. Retorna un int, no un char, 
    // porque necesita devolver -1 (EOF) que está fuera del rango de chars.
    while ((c = fgetc(in)) != EOF) {
        // ¿Deberíamos numerar la línea no estando en medio de una y dándose las opciones correctas?
        if (at_line_start) {
            bool is_blank = (c == '\n');
            
            if (opt_number_nonblank) {
                if (!is_blank) {
                    printf("%6d\t", line_counter++);
                }
            } else if (opt_number) {
                 printf("%6d\t", line_counter++);
            }
        }
        
        putchar(c);
        at_line_start = (c == '\n');
    }

    // Prevenimos error si stdin no es cerrado sino que hay errores de I/O en hardware.
    if (ferror(in)) {
        fprintf(stderr, "minicat: error leyendo de %s\n", filename);
        *exit_status = EXIT_FAILURE;
    }
}

int main(int argc, char *argv[]) {
    // 1. CONFIGURACIÓN DE OPCIONES
    static struct option long_options[] = {
        {"number", no_argument, 0, 'n'},
        {"number-nonblank", no_argument, 0, 'b'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "nb", long_options, NULL)) != -1) {
        switch (opt) {
            case 'n': opt_number = true; break;
            case 'b': 
                opt_number_nonblank = true; 
                opt_number = true; // -b anula -n y asume su rol mejorado 
                break;
            default:
                exit(EXIT_FAILURE); // getopt ya imprimió error
        }
    }

    int exit_status = EXIT_SUCCESS;

    // 2. LECTURA DE STREAM ESTÁNDAR (Comando limpio)
    // Si no le pasamos NINGÚN archivo, de hecho tiene que leerte el teclado!
    if (optind == argc) {
        cat_file(stdin, "stdin", &exit_status);
        return exit_status;
    }

    // 3. TRAYECTO MULTI-ARCHIVO
    for (int i = optind; i < argc; i++) {
        // Convención POSIX universal: un "-" significa "lee la entrada estandar" aquí mismo.
        if (strcmp(argv[i], "-") == 0) {
            cat_file(stdin, "stdin", &exit_status);
        } else {
            // Intentamos abrirlo, sin temor al NULL porque cat_file sabe manejar un !in pasándole filename y controlándole la basura.
            FILE *f = fopen(argv[i], "r");
            cat_file(f, argv[i], &exit_status);
            if (f) {
                fclose(f);
            }
        }
    }

    return exit_status;
}
