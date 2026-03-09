#include <stdio.h>
#include <stdlib.h>


int matar_dividiendo() {
    volatile int b = 0; // Usar volatile evita que GCC Optimice y quite la division 0
    // TODO: division / 0
    return b;
}

void matar_magia_os() {
    // TODO: char *m = NULL; mutar *m o strcpy(m, "A") ! SegFault Muerte atroz garantizada.
}

int main(int argc, char *argv[]) {
    // Escoger Muerte 1 o 2 
    return EXIT_SUCCESS;
}
