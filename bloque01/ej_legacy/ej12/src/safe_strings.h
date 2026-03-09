#ifndef SAFE_STRINGS_H
#define SAFE_STRINGS_H

#include <stddef.h>
#include <stdarg.h> // Para variadic arguments en snprintf

// Copia src a dst de forma segura.
// Retorna la longitud original de src.
size_t safe_strcpy(char *dst, const char *src, size_t dst_size);

// Concatena src en dst de forma segura.
// Retorna la longitud que el string habría tenido si el buffer fuese infinito.
size_t safe_strcat(char *dst, const char *src, size_t dst_size);

// Wrapper seguro alrededor de snprintf
int safe_snprintf(char *dst, size_t dst_size, const char *format, ...);

#endif
