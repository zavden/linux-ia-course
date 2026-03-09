# Ejercicio 1.2 — Strings y Buffers Seguros

## 🎯 Objetivo
Entender por qué funciones de C clásico como `strcpy` o `strcat` son peligrosas, y crear alternativas seguras que evitan Buffer Overflows (Desbordamiento de búfer), basándonos en cómo lo hacen OpenBSD y herramientas modernas.

## 📚 Teoría Mínima
En C, un *string* es solo una porción de memoria continua de caracteres que **debe** terminar en un terminador nulo `\0`.
- `strcpy(dest, src)` asume ciegamente que en `dest` hay espacio suficiente para todo `src`. Si no lo hay, sobreescribirá la memoria adyacente, corrompiendo datos o permitiendo ataques de inyección de código.
- `strncpy` fue creada para directorios, no para strings: si el string es más largo que el límite, **no** le pone el `\0` al final, dejando una "cadena abierta" muy peligrosa.
- Solución real: `strlcpy` y `strlcat` (nacieron en OpenBSD) o escribir tus propios wrappers seguros.

## 📝 Instrucciones

No vamos a inventar la rueda, pero vamos a entenderla. Escribe tu propia pequeña librería estática `safe_strings` en `src/`.

1. Crea `src/safe_strings.h` definiendo tres funciones:
   - `size_t safe_strcpy(char *dst, const char *src, size_t dst_size);`
   - `size_t safe_strcat(char *dst, const char *src, size_t dst_size);`
   - `int safe_snprintf(char *dst, size_t dst_size, const char *format, ...);`

2. Implementa estas funciones en `src/safe_strings.c`:
   - `safe_strcpy`: Copia de `src` a `dst` garantizando **siempre** que el último char sea `\0` (si `dst_size > 0`). Devuelve la longitud original de `src` (útil para detectar si hubo truncamiento). Puedes basarte en comportamiento de `strlcpy`.
   - `safe_strcat`: Concatena buscando primero el final actual de `dst` y rellenando lo que quepa garantizando el `\0`.
   - `safe_snprintf`: Es un wrapper. Haz que llame al `snprintf` estándar de `stdio.h` y devuelva los bytes impresos o un error.

3. Usa tu librería en `src/main.c` probando intentar desbordar un buffer intencionalmente y demostrando que tus funciones lo previenen.

## ✅ Criterios de Éxito
- Has creado código donde intentar copiar un string de 100 bytes a un buffer de 10 bytes no provoca una violación de segmento (Segfault) ni abortos del stack protector, sino que simplemente se trunca limpiamente con un terminador nulo al final (en la posición 9).
