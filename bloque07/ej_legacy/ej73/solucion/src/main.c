/*
 * Ejercicio 7.3 — Main (Dependencia a Make)
 */
#include <stdio.h>
#include <stdlib.h>
#include "mates.h"

int main(void) {
    printf("=== LIBRERÍA MATH COMPILADA SEPARADA C ===\n");
    printf(" 10 + 50 = %d\n", m_sumar(10, 50));
    printf(" 9 * 9 = %d\n", m_multi(9, 9));
    
    return EXIT_SUCCESS;
}
