#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Macro utilitaria para calcular la diferencia en ns de struct timespec
#define DIFF_IN_NS(end, start)                                                 \
  (((end).tv_sec - (start).tv_sec) * 1000000000ULL +                           \
   ((end).tv_nsec - (start).tv_nsec))

// Función tonta para simular trabajo (quemar CPU)
void cpu_heavy_task(void) {
  volatile long long sum =
      0; // volatile para que el compilador no quite el bucle por O3
  for (int i = 0; i < 10000000; i++) {
    sum += i;
  }
}

int main(void) {
  struct timespec start, end;

  // Empezamos a medir (CLOCK_MONOTONIC, NO CLOCK_REALTIME)
  if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
    perror("clock_gettime");
    return EXIT_FAILURE;
  }

  // El trabajo a medir
  printf("Iniciando una tarea pesada...\n");
  cpu_heavy_task();
  printf("Tarea pesada completada. Midiendo...\n");

  // Fin de la medición
  if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) {
    perror("clock_gettime");
    return EXIT_FAILURE;
  }

  // Calcular en nanosegundos el delta
  unsigned long long delta_ns = DIFF_IN_NS(end, start);

  // Transformar a milisegundos para print (ns / 1_000_000)
  double delta_ms = delta_ns / 1000000.0;

  // Transformar a segundos para print (ns / 1_000_000_000)
  double delta_sec = delta_ns / 1000000000.0;

  printf("Tiempo: %.6f segundos | %.3f milisegundos | %llu nanosegundos\n",
         delta_sec, delta_ms, delta_ns);

  return EXIT_SUCCESS;
}
