/*
 * Ejercicio 4.4 — Cadenas OS y Límites (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Introducir Programatic Limits (Setrlimit). 
 * Se muestra que la Memoria se deniega controladamente con `malloc=NULL` 
 * (sin matar a tu aplicacion abruptamente con el OOM), permitiendote  
 * retroceder, denegar peticiones de Servidor con "500 Server Busy" y seguir  
 * viviendo sin que se caiga tu NGINX/Apache C propio de Linux!.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h> // Set/Get r_limits, RLIMIT_ macros
#include <unistd.h>
#include <string.h>

#define MB_TALLA_BOMBA (1024 * 1024 * 1) // Bloques crudos exactos de 1 MB.

int main(void) {
    // 1. ESPIONAJE DEL ENTORNO OS DE PARTIDA
    struct rlimit l_archivos;
    if (getrlimit(RLIMIT_NOFILE, &l_archivos) == 0) {
        printf("I) Límites FileDescriptors Crudos Heredados de OS| Soft: %lu, Duro Absoluto Máximo: %lu\n", 
            (unsigned long)l_archivos.rlim_cur,  
            (unsigned long)l_archivos.rlim_max);
    } else { perror("Sondeo rlimit FDs bloqueado/fail"); }

    struct rlimit l_ram;
    if (getrlimit(RLIMIT_AS, &l_ram) == 0) {
        printf("II) Maximo de Memoria Virtual Exigible | Soft: ");
        if (l_ram.rlim_cur == RLIM_INFINITY) printf("INFINITO, ");
        else printf("%lu Bytes, ", (unsigned long)l_ram.rlim_cur);
        
        printf("Hard: ");
        if (l_ram.rlim_max == RLIM_INFINITY) printf("INFINITO\n");
        else printf("%lu Bytes\n", (unsigned long)l_ram.rlim_max);
    }

    // 2. AUTO-MUTILACIÓN DEL PROGRAMA MÁRTIR.
    printf("\nIII) ¡Activando Cinturón Asfixiante de Límite de la Memoria Total del Proceso a tan solo 5 MB Libres de tope virtual OS Kernell!\n");
    
    // Devolvemos el Hard Max al original del root (infinito quizas), pero le capamos el Soft Cur a 5 megaz (Asfixia). 
    struct rlimit suicidio_ram;
    suicidio_ram.rlim_cur = 5 * 1024 * 1024; //  (5 MB)
    suicidio_ram.rlim_max = l_ram.rlim_max;  //  (Infinito, o limite del systemd real, para no capar mi root original futuro)

    /*
     * EL CONTRATO OS:
     * Si Linux lo traga en Setrlimit, a partir de aca nuestra invocación en glibc
     * de `malloc/mmap` chocará y tronará sin avisar al sobre-alojar bytes por un miserable if 
     * arrojando ENOMEM desde el abismo de las System calls.
     */
    if (setrlimit(RLIMIT_AS, &suicidio_ram) != 0) {
        perror("Error critico. Tu OS O tu C++ no soportan la autocastración SETRLIMIT de Address Space POSIX");
        return EXIT_FAILURE;
    }

    // 3. BOMBARDEADOR LOOP KAMIKAZE.
    int exitos = 0;
    void *historial_pointers[20]; // Pointers al olvido local C

    for (int i = 0; i < 20; i++) {
        // Pedimos tajadas abusivas crueles y absurdas de un MegaBite continuo limpio.
        void *p = malloc(MB_TALLA_BOMBA);

        // Control OOM Resiliente 
        // ¡Este if es lo que nos salva de un crash (Segmentation Fault de deref NULL) 
        // Si tu memoria revienta y tu aplicacion lo caza formal acatráz con if, tú ganas!
        if (p == NULL) {
             printf("[!] BOMBARDEO ABORTADO EN LA VUELTA %d !.\n", i + 1);
             printf("   [X] Linux nos cortó la luz de golpe! Malloc denegado (NULL PTR Catch!). \n");
             printf("   [X] Almacenados con exito maximo: %d MB.\n", exitos);
             break;
        }
        
        // Pisar los bytes sueltos C. (Imprescindible por magia OS del Zero-Page Lazy Loading de kernel posix si queremos agotar ram real). 
        memset(p, 'Z', MB_TALLA_BOMBA);

        historial_pointers[i] = p; // Archivar en C
        exitos++;
        printf("   - Tajada De Basura MB Asignada Ok: %d\n", i + 1);
    }
    
    // 4. CONCLUSIÓN MILAGROSA. 
    printf("\n[FINAL] Tu codigo Demonio C NO MURIO ABOTADO a traicion por Linux (Osease, no arrojó la fatal de Kernell 'Killed').\n");
    printf("[FINAL] Supo rendirse bajo el Control Administrativo limitante y devolviendo Error 500 al cliente en paz!!!\n");

    // Reciclaje de los MB robados al C.
    for (int j = 0; j < exitos; j++) {
        free(historial_pointers[j]);
    }
    
    return EXIT_SUCCESS;
}
