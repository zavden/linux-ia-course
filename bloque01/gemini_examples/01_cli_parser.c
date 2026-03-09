#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void print_help(const char *prog_name) {
  printf("Uso: %s [OPCIONES]\n", prog_name);
  printf("Opciones:\n");
  printf("  -h, --help       Muestra esta ayuda y termina.\n");
  printf("  -v, --verbose    Activa la salida verbosa.\n");
  printf("  -o, --output=OUT Escribe el resultado en el archivo OUT.\n");
  printf("  -c, --count=N    Número de repeticiones (por defecto 1).\n");
}

int main(int argc, char *argv[]) {
  int opt;
  bool verbose = false;
  char *output_file = NULL;
  int count = 1;

  // Estructura de opciones largas
  static struct option long_options[] = {{"help", no_argument, 0, 'h'},
                                         {"verbose", no_argument, 0, 'v'},
                                         {"output", required_argument, 0, 'o'},
                                         {"count", required_argument, 0, 'c'},
                                         {0, 0, 0, 0}};

  int option_index = 0;

  // Procesar iterativamente cada flag
  while ((opt = getopt_long(argc, argv, "hvo:c:", long_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 'h':
      print_help(argv[0]);
      return EXIT_SUCCESS;
    case 'v':
      verbose = true;
      break;
    case 'o':
      output_file = optarg;
      break;
    case 'c':
      // Nota: en un código real, usar strtol para manejo robusto de errores
      count = atoi(optarg);
      if (count <= 0) {
        fprintf(stderr, "Error: count debe ser mayor a 0.\n");
        return EXIT_FAILURE;
      }
      break;
    case '?':
      // getopt_long ya imprime un mensaje de error
      return EXIT_FAILURE;
    default:
      abort();
    }
  }

  // Código principal de tu programa
  if (verbose) {
    printf("[INFO] Iniciando programa en modo verboso.\n");
    printf("[INFO] Repeticiones configuradas: %d\n", count);
    if (output_file) {
      printf("[INFO] Archivo de salida: %s\n", output_file);
    } else {
      printf("[INFO] Salida por consola.\n");
    }
  }

  // Argumentos posicionales (los que no son flags)
  for (int i = optind; i < argc; i++) {
    for (int j = 0; j < count; j++) {
      printf("Procesando argumento posicional: %s\n", argv[i]);
    }
  }

  return EXIT_SUCCESS;
}
