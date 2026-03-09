#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Wrapper variádico.
 * El contrato es igual al de snprintf/vsnprintf.
 */
static int safe_snprintf(char *dst, size_t dst_size, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int result = vsnprintf(dst, dst_size, fmt, ap);
    va_end(ap);
    return result;
}

int main(void) {
    char out[10];

    /* Forzamos texto largo para observar truncamiento de forma segura */
    int r = safe_snprintf(out, sizeof(out), "id=%d user=%s", 42, "debora");

    printf("out='%s'\n", out);
    printf("ret=%d\n", r);
    printf("truncated=%s\n", (r < 0 || (size_t)r >= sizeof(out)) ? "yes" : "no");

    return EXIT_SUCCESS;
}
