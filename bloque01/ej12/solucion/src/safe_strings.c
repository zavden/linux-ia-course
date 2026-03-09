/*
 * Implementación de librerías seguras (SOLUCIÓN DIDÁCTICA)
 */
#include "safe_strings.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/*
 * safe_strcpy:
 * Copia 'src' hacia 'dst'. Copia como máximo (dst_size - 1) caracteres.
 * Siempre remata con un '\0' si dst_size > 0.
 *
 * Retorna: 
 * La longitud real del string 'src'. Si la longitud retornada es >= dst_size, 
 * significa que ocurrió un truncamiento (no cupo). El desarrollador puede chequear 
 * ese retorno y saber si falló el buffer en vez de un silencioso overflow.
 */
size_t safe_strcpy(char *dst, const char *src, size_t dst_size) {
    size_t src_len = strlen(src);
    
    // Solo podemos trabajar si nos dieron al menos 1 byte en destino
    if (dst_size > 0) {
        size_t copy_len = src_len;
        
        // Si el src real es mayor o igual al espacio total, limitamos a la capacidad real - 1 (para el \0)
        if (copy_len >= dst_size) {
            copy_len = dst_size - 1;
        }
        
        // Copia exacta y ciega de 'copy_len' bytes
        memcpy(dst, src, copy_len);
        
        // Final nulo manual garantizado
        dst[copy_len] = '\0';
    }
    
    return src_len;
}

/*
 * safe_strcat:
 * Concatena 'src' al final del contenido actual de 'dst'.
 * Garantiza que la combinación resultante nunca excederá 'dst_size' (incluyendo \0).
 */
size_t safe_strcat(char *dst, const char *src, size_t dst_size) {
    // strlen() del string original en el destino y del pedazo nuevo.
    size_t dst_len = strlen(dst);
    size_t src_len = strlen(src);
    size_t i;

    // Si el destino ya estaba lleno o estropeado (supera su propio size reportado), 
    // abortamos la copia y reportamos el tamaño requerido para que se sepa.
    if (dst_len >= dst_size) {
        return dst_size + src_len;
    }

    // Calcula cuántos bytes nos sobran como máximo para la parte 'src'.
    size_t room_left = dst_size - dst_len - 1;
    size_t copy_len = src_len < room_left ? src_len : room_left;

    // i copia uno a uno desde donde se quedó el string de destino.
    for (i = 0; i < copy_len; i++) {
        dst[dst_len + i] = src[i];
    }
    
    // Terminador garantizado en la última posición reescrita
    dst[dst_len + i] = '\0';

    return dst_len + src_len; 
}

/*
 * safe_snprintf:
 * Wrappera el tradicional snprintf, que evalúa interpolaciones con formato ("%s %d").
 * Se usa <stdarg.h> para rutear los argumentos variables (...) hacia vsnprintf.
 */
int safe_snprintf(char *dst, size_t dst_size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    // vsnprintf también es seguro nativamente (a diferencia de sprintf puro)
    // Pero lo incluímos como estándar de nuestra propia librería.
    int ret = vsnprintf(dst, dst_size, format, args);
    
    va_end(args);
    return ret;
}
