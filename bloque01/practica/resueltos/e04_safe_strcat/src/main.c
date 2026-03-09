#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * safe_strcat:
 * - No asume que dst tenga espacio infinito
 * - Respeta dst_size
 * - Retorna longitud objetivo (dst_len inicial + src_len)
 */
static size_t safe_strcat(char *dst, const char *src, size_t dst_size) {
    size_t dst_len = strnlen(dst, dst_size);
    size_t src_len = strlen(src);

    /* Si dst ya no tiene terminador en rango, no podemos concatenar seguro */
    if (dst_len == dst_size) {
        return dst_size + src_len;
    }

    /* Espacio útil real dejando 1 byte para '\0' */
    size_t free_n = dst_size - dst_len - 1;
    size_t copy_n = (src_len < free_n) ? src_len : free_n;

    memcpy(dst + dst_len, src, copy_n);
    dst[dst_len + copy_n] = '\0';

    return dst_len + src_len;
}

int main(void) {
    char dst[12] = "Hola";
    size_t wanted = safe_strcat(dst, " Mundo Extenso", sizeof(dst));

    printf("dst='%s'\n", dst);
    printf("wanted=%zu\n", wanted);
    printf("truncated=%s\n", wanted >= sizeof(dst) ? "yes" : "no");

    return EXIT_SUCCESS;
}
