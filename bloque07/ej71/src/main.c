#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void f_leak() {
    // TODO: malloc sin free
}

void f_zombie() {
    // TODO: malloc. free. Y usar despues
}

void f_overflow() {
    // TODO: malloc de 10. Intentar mutar el ID 15 y 16 array. free(..)
}

int main(int argc, char *argv[]) {
    // int d = atoi(argv[1]);
    // ifs.. ejectuando funciones rotas intencionales C 
    return EXIT_SUCCESS;
}
