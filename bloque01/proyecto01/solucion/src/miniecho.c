/*
 * Proyecto 1 — miniecho (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Recrear el comando `echo` de GNU Coreutils desde cero en C estándar.
 * Nos enseña a:
 * - Iterar correctamente sobre `argv` saltando el nombre del propio programa.
 * - Detectar modificadores puramente posicionales (sin getopt) como se 
 *   espera históricamente de `echo`.
 * - Parsear y traducir literales de texto escapado (ej: "\n" de 2 char a un 
 *   auténtico '\n' de 1 char).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/*
 * process_escapes:
 * Función que lee un string crudo (raw) buscando la combinación '\' + 'letra'.
 * Si la encuentra, imprime el caracter ASCII de control real en lugar de los 2 caracteres.
 */
void process_escapes(const char *str, bool *abort_output) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        // ¿Estamos ante una barra invertida?
        if (str[i] == '\\' && str[i+1] != '\0') {
            i++; // Miramos el siguiente caracter para decidir
            switch (str[i]) {
                case 'n': putchar('\n'); break;
                case 't': putchar('\t'); break;
                case '\\': putchar('\\'); break;
                case 'c': 
                    *abort_output = true; // El flag aborta el resto del programa
                    return;
                default:
                    // Si no es un escape soportado, lo imprimimos tal cual (\, luego la letra)
                    putchar('\\');
                    putchar(str[i]);
                    break;
            }
        } else {
            // Un caracter normal, simplemente se "escupir" a la pantalla
            putchar(str[i]);
        }
    }
}

int main(int argc, char *argv[]) {
    bool no_newline = false;
    bool enable_escapes = false;
    int index = 1;

    // 1. ANÁLISIS DE MODIFICADORES (PARSE)
    // Históricamente, 'echo' no usa getopt rígido. Revisa los primeros argumentos
    // hasta encontrar uno que no sea -n o -e o -E.
    while (index < argc) {
        if (strcmp(argv[index], "-n") == 0) {
            no_newline = true;
        } else if (strcmp(argv[index], "-e") == 0) {
            enable_escapes = true;
        } else if (strcmp(argv[index], "-E") == 0) {
            enable_escapes = false; // El por defecto de POSIX, la opción contraria
        } else {
            // Ya no es modificador válido, es el principio del texto
            break;
        }
        index++;
    }

    // Bandera para la opción secreta "\c"
    bool abort_output = false;

    // 2. IMPRESIÓN TEXTUAL
    for (int i = index; i < argc; i++) {
        // Si nos mandaron abortar antes, cortamos el loop instantáneamente
        if (abort_output) break;

        if (enable_escapes) {
            process_escapes(argv[i], &abort_output);
        } else {
            // Sin escape: usa lo estándar y rápido de stdio
            fputs(argv[i], stdout);
        }

        // 3. ESPACIADO
        // Los argumentos en bash de `echo hola mundo` vienen separados .
        // Lo imprimimos para simular el original, SOLO si no es el último.
        if (i < argc - 1 && !abort_output) {
            putchar(' ');
        }
    }

    // 4. NUEVA LÍNEA FINAL (El corazón de echo)
    if (!no_newline && !abort_output) {
        putchar('\n');
    }

    // Aseguramos que el contenido del búfer del kernel se vacía a la shell del usuario.
    fflush(stdout);
    
    return EXIT_SUCCESS;
}
