/*
 * Ejercicio 1.5 — Aritmética de Punteros (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Demostrar por qué JAMÁS debemos reinventar la rueda en C para operaciones masivas de memoria.
 * `memcpy` original usa rutinas directamente programadas en Ensamblador y registros
 * SIMD/AVX de las CPU modernas (128, 256 o 512 bits) para copiar docenas de bytes 
 * en una sola instrucción de hardware, destrozando cualquier bucle for que puedas escribir.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>   // Para clock_gettime

/*
 * mi propia reimplementación de memcpy.
 * void* asegura poder recibir int arrays, structs o lo que sea.
 */
void *my_memcpy(void *dest, const void *src, size_t n) {
    // 1. Casteamos forzosamente los void* a un tipo definido byte-por-byte (char*).
    // Si no lo hacemos, no se permite aritmética como `d++` (el compilador no sabe cuánto es 1 void).
    char *d = (char *)dest;
    const char *s = (const char *)src;
    
    // 2. Bucle ingenuo, copiando 1 byte a la vez por N veces.
    // Esto es muy costoso para el CPU.
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

/* 
 * Helper: Calcula el delta en milisegundos reales con punto flotante.
 */
double diff_ms(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 + (end->tv_nsec - start->tv_nsec) / 1000000.0;
}

int main(void) {
    // Alojar 50 MB de basurilla
    size_t size = 50 * 1024 * 1024;
    
    char *src = malloc(size);
    char *dst = malloc(size);
    
    if(!src || !dst) {
        perror("Fallo malloc");
        exit(1);
    }
    
    // Llenar de datos iniciales. `memset` es otra primitiva ultraoptimizada
    memset(src, 0xAB, size);

    struct timespec start, end;
    
    // ============================================
    // CASO 1: Nuestro cutre my_memcpy
    // ============================================
    clock_gettime(CLOCK_MONOTONIC, &start);
    my_memcpy(dst, src, size);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double t_manual = diff_ms(&start, &end);
    printf("my_memcpy (Bucle Manual) tardo: %f ms\n", t_manual);

    // ============================================
    // CASO 2: La bestial API Pura standard de C
    // ============================================
    clock_gettime(CLOCK_MONOTONIC, &start);
    memcpy(dst, src, size);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double t_nativo = diff_ms(&start, &end);
    printf("memcpy nativo tardo       : %f ms\n", t_nativo);

    // Conclusión final y factor multiplicador
    if (t_nativo > 0) {
        printf("CONCLUSION: memcpy nativo es %.2f veces mas rapido.\n", t_manual / t_nativo);
    }

    free(src);
    free(dst);

    return EXIT_SUCCESS;
}
