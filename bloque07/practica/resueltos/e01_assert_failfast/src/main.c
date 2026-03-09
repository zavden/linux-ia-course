#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Invariante de dominio:
 * min siempre debe ser <= max.
 */
static int clamp_checked(int value, int min, int max) {
    /* Fail-fast para errores de programación (contrato interno roto). */
    assert(min <= max);

    /* Normalizamos por límite inferior/superior de forma explícita. */
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

/*
 * División segura:
 * evita UB de división por cero devolviendo código de error.
 */
static int divide_safe(int a, int b, int *out) {
    /* out no debe ser NULL: invariante interna del API. */
    assert(out != NULL);

    /* Validación de entrada externa: no depende de assert. */
    if (b == 0) {
        return -1;
    }

    *out = a / b;
    return 0;
}

int main(void) {
    /* Caso de clamp: 150 en rango [0,100] debe saturar en 100. */
    int r = clamp_checked(150, 0, 100);

    int q = 0;
    /* Primera división válida. */
    int ok1 = divide_safe(20, 5, &q);
    /* Segunda división inválida para probar ruta de error controlada. */
    int ok2 = divide_safe(20, 0, &q);

    /* Salida estable para test automatizado. */
    printf("clamp=%d div_ok=%d div_err=%d q=%d\n", r, ok1, ok2, q);

    return (r == 100 && ok1 == 0 && ok2 == -1 && q == 4) ? EXIT_SUCCESS : EXIT_FAILURE;
}
