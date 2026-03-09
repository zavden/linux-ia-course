/*
 * Ejercicio 1.1 — Argumentos y getopt (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Las utilidades profesionales de Linux no analizan `argv` con bucles `for`
 * y `strcmp()`. Utilizan APIs estándar POSIX como `getopt` o su extensión GNU
 * `getopt_long` para un parseo uniforme, soportando sintaxis como:
 * `./app -v -n15` o `./app --verbose --number=15`.
 */

#include <stdio.h>   // Para printf, fprintf
#include <stdlib.h>  // Para exit, atoi, EXIT_SUCCESS, EXIT_FAILURE
#include <getopt.h>  // API central para parseo de argumentos
#include <stdbool.h> // Para el tipo bool (C99)

/*
 * 1. FUNCIÓN DE AYUDA (Help)
 * Siempre es buena práctica imprimir cómo se usa el programa si el usuario
 * solicita ayuda o si introduce argumentos inválidos.
 */
void print_help(const char *prog_name) {
    printf("Uso: %s [OPCIONES]\n", prog_name);
    printf("Opciones:\n");
    printf("  -v, --verbose       Activa modo ruidoso o debug.\n");
    printf("  -o, --output=FILE   Escribe salida en FILE (default: salida.txt).\n");
    printf("  -n, --number=NUM    Configura el limite numérico a NUM (default: 0).\n");
    printf("  -h, --help          Muestra este mensaje de ayuda y termina.\n");
}

int main(int argc, char *argv[]) {
    /* 
     * 2. ESTADO INICIAL (Fallbacks)
     * Las variables deben tener valores razonables por defecto por si el
     * el usuario no pasa ninguna bandera opcional.
     */
    bool verbose = false;
    const char *output_file = "salida.txt";
    int number = 0;

    /* 
     * 3. DEFINICIÓN DE OPCIONES LARGAS (struct option array)
     * Requerido exclusivamente para getopt_long().
     * Estructura: { "nombre_largo", tiene_argumento, puntero_flag, val_equivalente_corto }
     * 
     * - no_argument: No toma valor (ej: --verbose).
     * - required_argument: Toma valor obligatoriamente (ej: --output=archivo.txt).
     * - El array DEBE terminar obligatoriamente con un elemento lleno de ceros puros.
     */
    static struct option long_options[] = {
        {"verbose", no_argument,       0, 'v'},
        {"output",  required_argument, 0, 'o'},
        {"number",  required_argument, 0, 'n'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    /* 
     * 4. EL BUCLE DE PARSEO
     * getopt_long lee argumentos del array `argv`.
     * Retorna el short-character ('v', 'o', etc.) si encontró un match, o -1 si ya acabó.
     * Retorna '?' si el usuario inyecta una bandera inválida.
     * 
     * El tercer argumento string ("vo:n:h") indica:
     * - v: Toma una -v sin más.
     * - o:: Con dos puntos, significa "lleva argumento obligatorio" -> -o ARCHIVO.
     * - n:: Lleva argumento obligatorio.
     * - h: Sin argumentos.
     */
    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "vo:n:h", long_options, &option_index)) != -1) {
        
        // Evaluamos QUÉ bandera encontró en esta iteración del bucle.
        switch (opt) {
            case 'v':
                verbose = true;
                break;
                
            case 'o':
                // `optarg` es la variable global mágica (en string) donde
                // getopt guarda el valor que iba pegado a '-o' o '--output='
                output_file = optarg;
                break;
                
            case 'n':
                // `atoi` (Ascii To Integer) convierte el string en un int.
                // Podría mejorarse con strtol() en un escenario muy seguro.
                number = atoi(optarg);
                break;
                
            case 'h':
                // Se nos pidió ayuda. Imprimimos el formato y salimos "con éxito".
                print_help(argv[0]);
                exit(EXIT_SUCCESS);
                
            case '?':
                // Opt inválida o argumento faltante de una válida.
                // getopt ya habrá impreso en stderr su propio enojo nativo.
                print_help(argv[0]);
                exit(EXIT_FAILURE);
                
            default:
                // Abortar genérico por si ocurre algo imposible.
                abort();
        }
    }

    /*
     * 5. ARGUMENTOS POSICIONALES RESTANTES
     * Si el comando es: `./app -v archivo1.txt archivo2.txt`
     * Los archivos finales NO son opciones (no tienen '-'). 
     * getopt acomoda esos argumentos al final del array argv,
     * a partir del índice `optind`.
     */
    if (optind < argc) {
        printf("Detectados argumentos posicionales adicionales ignorados:\n");
        while (optind < argc) {
            printf("  %s\n", argv[optind++]);
        }
    }

    /*
     * 6. IMPRESIÓN DEMOSTRATIVA DE ESTADO FINAL
     */
    printf("=== CONFIGURACIÓN FINAL ===\n");
    printf("Verbose: %s\n", verbose ? "ON" : "OFF");
    printf("Output : %s\n", output_file);
    printf("Number : %d\n", number);

    return EXIT_SUCCESS;
}
