#include <stdio.h>
#include <stdlib.h>

/*
 * Sumatoria 1..n con dos implementaciones:
 * - normal: loop explícito
 * - fast: fórmula cerrada
 */
static long sum_1_to_n(int n) {
#if defined(FAST_MODE)
    return (long)n * (n + 1) / 2;
#else
    long acc = 0;
    for (int i = 1; i <= n; ++i) {
        acc += i;
    }
    return acc;
#endif
}

int main(void) {
    /* Ejecutamos una sola entrada para comparar comportamiento entre modos. */
    long s = sum_1_to_n(100);

#if defined(FAST_MODE)
    int fast = 1;
#else
    int fast = 0;
#endif

    /* En build por defecto FAST_MODE no está definido => fast=0. */
    /* Para probar fast=1: compilar con -DFAST_MODE=1 en CFLAGS. */
    printf("fast_mode=%d sum=%ld\n", fast, s);
    /* El resultado debe ser idéntico sin importar la ruta elegida. */
    return (fast == 0 && s == 5050) ? EXIT_SUCCESS : EXIT_FAILURE;
}
