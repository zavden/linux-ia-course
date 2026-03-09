#include <stdio.h>
#include <stdlib.h>

#include "calc.h"
#include "fmt.h"

int main(void) {
    /* main solo orquesta; la lógica vive en módulos separados. */
    /* Si falla el link o include, este ejercicio lo revela enseguida. */
    int a = calc_add(7, 5);
    int s = calc_sub(7, 5);

    char msg1[64];
    char msg2[64];

    /* Formateamos resultados en texto sin duplicar lógica en main. */
    if (fmt_pair(msg1, sizeof(msg1), "sum", a) == -1 ||
        fmt_pair(msg2, sizeof(msg2), "sub", s) == -1) {
        fprintf(stderr, "fmt_pair failed\n");
        return EXIT_FAILURE;
    }

    /* Salida compacta para comprobar integración de ambos módulos. */
    printf("%s %s\n", msg1, msg2);
    /* Resultado esperado fijo para test automático. */
    return (a == 12 && s == 2) ? EXIT_SUCCESS : EXIT_FAILURE;
}
