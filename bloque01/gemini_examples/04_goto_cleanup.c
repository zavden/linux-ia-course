#include <stdio.h>
#include <stdlib.h>

/*
 * Objetivo: Leer dos archivos simulados, asignar algo de memoria y consolidar
 * todo el limpieza al final sin duplicar código ni arriesgarnos a un memory
 * leak en retornos intermedios tempranos.
 */

// Estas macros simplifican la lectura dentro del cuerpo
#define GOTO_ERROR(msg)                                                        \
  do {                                                                         \
    fprintf(stderr, "Error: %s\n", msg);                                       \
    status = EXIT_FAILURE;                                                     \
    goto cleanup;                                                              \
  } while (0)

int process_data(const char *in_path1, const char *in_path2) {
  int status = EXIT_SUCCESS;

  // 1. Inicializar TODO lo que necesita liberación al principio (a NULL)
  FILE *f1 = NULL;
  FILE *f2 = NULL;
  char *internal_buffer = NULL;

  printf("1. Abriendo archivo 1...\n");
  f1 = fopen(in_path1, "r");
  if (!f1)
    GOTO_ERROR("No se pudo abrir archivo 1");

  printf("2. Asignando memoria...\n");
  internal_buffer = malloc(1024);
  if (!internal_buffer)
    GOTO_ERROR("Error de memoria (malloc)");

  printf("3. Abriendo archivo 2...\n");
  f2 = fopen(in_path2, "r");
  // Simularemos un fallo en la apertura de f2 si la ruta es "FAIL"
  if (!f2)
    GOTO_ERROR("No se pudo abrir archivo 2");

  // Lógica del programa exitosa...
  printf("Procesamiento completado con éxito.\n");

// PUNTO DE SALIDA ÚNICO
cleanup:
  printf("\n--- Cleanup --- \n");

  // 2. Liberar en orden inverso (o igual, no importa mientras lo chequees)
  if (f2) {
    printf(" -> Cerrando archivo 2\n");
    fclose(f2);
  }

  if (internal_buffer) {
    printf(" -> Liberando internal_buffer\n");
    free(internal_buffer);
    // Opcional: internal_buffer = NULL; si fueras a hacer más cosas...
  }

  if (f1) {
    printf(" -> Cerrando archivo 1\n");
    fclose(f1);
  }

  return status;
}

int main(void) {
  printf("=== PRUEBA EXITOSA ===\n");
  // Pasamos /dev/null porque siempre existe en Linux, simulando éxito
  process_data("/dev/null", "/dev/null");

  printf("\n=== PRUEBA FALLIDA ===\n");
  // El segundo archivo no existirá, debe fallar a medias
  process_data("/dev/null", "ARCHIVO_NO_EXISTENTE.FAIL");

  return 0;
}
