#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // 1. Validar argumentos
    // 2. Convertir string octal a mode_t
    // 3. Modificar la umask y guardarla
    // 4. Crear archivo con open() o usar chmod()
    // 5. Restaurar umask
    // 6. Hacer system("getfacl archivo")
    
    return EXIT_SUCCESS;
}
