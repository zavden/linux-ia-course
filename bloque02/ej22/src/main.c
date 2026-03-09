#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

void copy_syscall_1byte(const char *in, const char *out) {
    // TODO: Usar open() O_RDONLY, open() O_WRONLY...
    // char c; read(fd, &c, 1); write(fd2, &c, 1); ...
}

void copy_stdio_1byte(const char *in, const char *out) {
    // TODO: Usar fopen("r"), fopen("w")...
    // int c; c = fgetc(in_f); fputc(c, out_f); ...
}

int main(void) {
    const char *origen = "large_test_file.dat";
    const char *dest1 = "temp1.dat";
    const char *dest2 = "temp2.dat";

    // FIXME: Primero deberás asegurarte de tener bash dd o similar generar un archivo para probar.

    // 1. Syscalls puras - Prepárate para esperar...
    // printf("Iniciando modo Syscall a 1 byte (esto va a demorar...\n");
    // TODO: Tomar time, llamar copy_syscall_1byte, tomar end time.

    // 2. Stdio wrapper (Buffers transparentes)
    // printf("Iniciando modo Stdio... \n");
    // TODO: Tomar time, llamar copy_stdio_1byte, tomar end time.

    // TODO: Imprimir comparativa en MS.

    return EXIT_SUCCESS;
}
