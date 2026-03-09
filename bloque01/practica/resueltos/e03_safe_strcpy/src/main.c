#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * safe_strcpy:
 * - Copia src -> dst respetando dst_size
 * - Siempre termina con '\0' cuando dst_size > 0
 * - Retorna la longitud original de src
 */
static size_t safe_strcpy(char *dst, const char *src, size_t dst_size) {
    size_t src_len = strlen(src);

    /* Si no hay espacio en destino, solo informamos cuánto se requería */
    if (dst_size == 0) {
        return src_len;
    }

    /* Reservamos un byte para el terminador */
    size_t copy_n = (src_len < (dst_size - 1)) ? src_len : (dst_size - 1);

    /* Copiamos bytes útiles */
    memcpy(dst, src, copy_n);

    /* Cerramos string sí o sí */
    dst[copy_n] = '\0';

    return src_len;
}

int main(void) {
    char dst[8];

    /* Forzamos truncamiento intencional para verificar el contrato */
    size_t required = safe_strcpy(dst, "LinuxCourse", sizeof(dst));

    printf("dst='%s'\n", dst);
    printf("required=%zu\n", required);
    printf("truncated=%s\n", required >= sizeof(dst) ? "yes" : "no");

    return EXIT_SUCCESS;
}
