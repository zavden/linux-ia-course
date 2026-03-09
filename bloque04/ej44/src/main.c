#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>

int main(void) {
    // TODO: Usar struct rlimit r;
    // TODO: Llamar a getrlimit(RLIMIT_NOFILE, &r) y printearlo.
    // TODO: Llamar a getrlimit(RLIMIT_AS, &r)...
    
    // TODO: Mudar r.rlim_cur y r.rlim_max a 10 Megabytes por ej.
    // TODO: Aplicarlo pidiendo setrlimit al Kernel
    
    // TODO: Intentar consumir sin control la RAM (bucle malloc) 
    // Comprobar con ifs protegiendo si Malloc llega a null.
    // Si da null, !Logramos el auto-sabotaje del OS límite exitosamente!
    
    return EXIT_SUCCESS;
}
