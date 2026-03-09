#include <stdio.h>
#include <stdlib.h>

int main(void) {
#ifdef __VERSION__
    printf("[Multi-Distro Test] Compilado con GCC %s\n", __VERSION__);
#else
    printf("[Multi-Distro Test] Compilador desconocido\n");
#endif
    return EXIT_SUCCESS;
}
