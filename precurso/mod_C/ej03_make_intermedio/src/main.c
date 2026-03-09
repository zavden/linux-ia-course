#include "utils.h"
#include <stdio.h>
#include "math.h"

int main(void) {
    setup_logger();
    int res = sumar(40, 2);
    printf("El resultado de 40 + 2 es: %d\n", res);
    return 0;
}
