#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

void print_help(const char *prog_name) {
    // TODO: Implementar mensaje de ayuda
}

int main(int argc, char *argv[]) {
    // Variables por defecto
    int verbose = 0;
    char *output_file = "salida.txt";
    int number = 0;

    // TODO: Usar getopt_long para iterar sobre todos los argumentos
    // TODO: Parsear -v, -o, -n, -h y sus versiones largas.
    
    // Al final, imprimir el resultado:
    printf("Verbose: %s\n", verbose ? "ON" : "OFF");
    printf("Output: %s\n", output_file);
    printf("Number: %d\n", number);

    return EXIT_SUCCESS;
}
