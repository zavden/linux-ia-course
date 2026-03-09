/*
 * Ejercicio 2.2 — Syscalls vs Stdio (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Demostrar empirícamente el concepto de Context Switch Penalty.
 * Un Context Switch (cambio de contexto de tu app Usuario al Kernel para 
 * tocar el disco) es de las cosas más costosas de un Sistema Operativo.
 * fopen/fgetc ocultan esto manejando "chunks" gigantes internamente.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

double diff_ms(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 + (end->tv_nsec - start->tv_nsec) / 1000000.0;
}

void copy_syscall_1byte(const char *in, const char *out) {
    int fd_in = open(in, O_RDONLY);
    int fd_out = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd_in < 0 || fd_out < 0) return;

    char c;
    /*
     * PELIGRO: Esto somete al CPU a un estrés brutal. 
     * Cada vuelta del bucle detiene tu app, levanta nivel de privilegios,
     * habla con el disco, baja privilegios, rellena 1 misero byte de RAM
     * y rutea el bucle.
     */
    while (read(fd_in, &c, 1) > 0) {
        if(write(fd_out, &c, 1) < 0) break;
    }

    close(fd_in);
    close(fd_out);
}

void copy_stdio_1byte(const char *in, const char *out) {
    FILE *f_in = fopen(in, "r");
    FILE *f_out = fopen(out, "w");
    if(!f_in || !f_out) return;

    int c;
    /*
     * MAGIA NEGRA: `fgetc` parece hacer lo mismo. Pero NO hace un syscall.
     * Solo extrae del búfer en RAM local usando punteros simples. 
     * Cuando se agota el búfer, the GLIBC hace UNA (1) syscall invisible 
     * gigante de varios Kilobytes al Kernel.
     */
    while ((c = fgetc(f_in)) != EOF) {
        fputc(c, f_out);
    }

    fclose(f_in);
    fclose(f_out);
}

int main(void) {
    const char *f_origen = "origen_bench.dat";
    const char *f_sys = "sys_out.dat";
    const char *f_std = "std_out.dat";

    // 1. Generar 2.5 MB de basura.
    // Con 2.5 millones de bytes, haremos literalmente 5 millones de syscalls (2.5 de read y 2.5 de write).
    printf("Generando 2.5MB de datos random...\n");
    system("dd if=/dev/urandom of=origen_bench.dat bs=1M count=2 status=none");
    system("dd if=/dev/urandom of=origen_bench.dat bs=512K count=1 oflag=append conv=notrunc status=none");


    struct timespec start, end;
    
    // --- Test SYSCALL (Muy Lento) ---
    printf("\nTest 1: Syscalls crudas a 1 Byte (Paciencia extrema, el OS esta sudando...)\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    copy_syscall_1byte(f_origen, f_sys);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double t_sys = diff_ms(&start, &end);
    printf("-> Tiempo Syscall: %.2f ms\n", t_sys);

    // --- Test STDIO (Muy Rápido) ---
    printf("\nTest 2: Libreria Estandar C (Buffered) a 1 Byte\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    copy_stdio_1byte(f_origen, f_std);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double t_std = diff_ms(&start, &end);
    printf("-> Tiempo Stdio: %.2f ms\n", t_std);

    if (t_std > 0) {
        printf("\n🔥 CONCLUSION: fgetc / stdio.h fue %.2f VECES MAS RAPIDO que read() directo.\n", t_sys / t_std);
    }

    // Limpieza
    unlink(f_origen);
    unlink(f_sys);
    unlink(f_std);

    return EXIT_SUCCESS;
}
