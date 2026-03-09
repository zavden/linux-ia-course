#include <stdio.h>
#include <stdlib.h>

static int tests_passed = 0;
static int tests_failed = 0;

/*
 * Macro de aserción simple para comparar enteros.
 * Acumula métricas sin abortar en el primer fallo.
 */
#define EXPECT_EQ(label, got, expected)                                      \
    do {                                                                      \
        int _got = (got);                                                     \
        int _exp = (expected);                                                \
        if (_got == _exp) {                                                   \
            tests_passed++;                                                   \
        } else {                                                              \
            tests_failed++;                                                   \
            printf("FAIL %s got=%d expected=%d\n", (label), _got, _exp);   \
        }                                                                     \
    } while (0)

static int add(int a, int b) { return a + b; }
static int mul(int a, int b) { return a * b; }

int main(void) {
    /* Suite mínima de pruebas para funciones puras. */
    /* En un proyecto real, cada bloque sería una función test separada. */
    EXPECT_EQ("add_1", add(2, 3), 5);
    EXPECT_EQ("add_2", add(-1, 1), 0);
    EXPECT_EQ("mul_1", mul(4, 5), 20);
    EXPECT_EQ("mul_2", mul(-2, 3), -6);

    /* Reporte final estilo harness simple. */
    printf("passed=%d failed=%d\n", tests_passed, tests_failed);

    /* Código de salida facilita usar este binario en CI/CD. */
    return (tests_passed == 4 && tests_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
