#include <stdio.h>
#include "info.h"

void info(void) {
    printf("=== Información de Build ===\n");
#ifdef __VERSION__
    printf("Compilador: %s\n", __VERSION__);
#endif
    printf("Fecha: %s %s\n", __DATE__, __TIME__);
#ifdef __STDC_VERSION__
    printf("Estándar C: %ld\n", __STDC_VERSION__);
#endif
    printf("============================\n");
}
