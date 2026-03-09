/*
 * Ejercicio 1.2 — Strings y Buffers Seguros (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Estas funciones imitan el comportamiento de la librería strlcpy/strlcat de OpenBSD.
 * A diferencia de strncpy, estas SIEMPRE agregan el terminador nulo '\0' al final,
 * garantizando que el string vuelva a ser válido en C y no cause lecturas fuera de rango.
 */
#ifndef SAFE_STRINGS_H
#define SAFE_STRINGS_H

#include <stddef.h> // size_t

size_t safe_strcpy(char *dst, const char *src, size_t dst_size);
size_t safe_strcat(char *dst, const char *src, size_t dst_size);
int safe_snprintf(char *dst, size_t dst_size, const char *format, ...);

#endif
