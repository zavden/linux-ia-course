#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* Diferencia en nanosegundos */
static long ns_diff(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1000000000L + (b.tv_nsec - a.tv_nsec);
}

/* Copia byte-a-byte con syscalls puras */
static int copy_sys_byte(const char *in_path, const char *out_path) {
    int in = -1, out = -1;
    int rc = -1;
    unsigned char c;

    in = open(in_path, O_RDONLY);
    if (in == -1) goto cleanup;

    out = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out == -1) goto cleanup;

    for (;;) {
        ssize_t r = read(in, &c, 1);
        if (r == 0) break;
        if (r == -1) goto cleanup;
        if (write(out, &c, 1) != 1) goto cleanup;
    }

    rc = 0;

cleanup:
    if (in != -1) close(in);
    if (out != -1) close(out);
    return rc;
}

/* Copia byte-a-byte con stdio (buffer interno) */
static int copy_stdio_byte(const char *in_path, const char *out_path) {
    FILE *in = NULL;
    FILE *out = NULL;
    int rc = -1;

    in = fopen(in_path, "rb");
    if (!in) goto cleanup;

    out = fopen(out_path, "wb");
    if (!out) goto cleanup;

    for (;;) {
        int ch = fgetc(in);
        if (ch == EOF) {
            if (feof(in)) break;
            goto cleanup;
        }
        if (fputc(ch, out) == EOF) goto cleanup;
    }

    rc = 0;

cleanup:
    if (in) fclose(in);
    if (out) fclose(out);
    return rc;
}

int main(void) {
    const char *src = "bench_src.dat";
    const char *a = "bench_sys.dat";
    const char *b = "bench_stdio.dat";

    /* Generamos archivo de 1 MiB para prueba rápida */
    FILE *f = fopen(src, "wb");
    if (!f) {
        perror("fopen src");
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < 1024 * 1024; ++i) {
        fputc((int)(i & 0xFF), f);
    }
    fclose(f);

    struct timespec t1, t2;

    clock_gettime(CLOCK_MONOTONIC, &t1);
    if (copy_sys_byte(src, a) != 0) {
        perror("copy_sys_byte");
        return EXIT_FAILURE;
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);
    long ns_sys = ns_diff(t1, t2);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    if (copy_stdio_byte(src, b) != 0) {
        perror("copy_stdio_byte");
        return EXIT_FAILURE;
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);
    long ns_stdio = ns_diff(t1, t2);

    printf("sys_byte_ms=%.3f\n", ns_sys / 1e6);
    printf("stdio_byte_ms=%.3f\n", ns_stdio / 1e6);

    return EXIT_SUCCESS;
}
