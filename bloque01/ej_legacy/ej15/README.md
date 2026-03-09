# Ejercicio 1.5 — Aritmética de Punteros y Benchmarks

## 🎯 Objetivo
Hacer una reimplementación de `memcpy`, `memmove` y `memset` para entender cómo manipular matrices brutas. Luego medir el rendimiento y asombrarte de cuán optimizadas están las bibliotecas de C en Linux con instrucciones SIMD de la CPU.

## 📚 Teoría Mínima
- `void *`: Es solo una dirección de memoria. El compilador no sabe cuántos bytes ocupa lo que hay ahí.
- **Aritmética de punteros**: Si haces `ptr + 1`, el compilador avanza N bytes dependiendo del tipo de `ptr`. Si es un `int *`, avanza 4 bytes. Por eso, para manipular memoria byte a byte con libertad total en utilidades, convertimos todo a `char *` ó `uint8_t *` (que ocupan 1 byte exacto).
- `clock_gettime(CLOCK_MONOTONIC, &tv)`: Funciones de Linux POSIX de alta resolución. Evita el simple `time()` porque es resolución de segundos e inestable ante sincronizaciones de NTP.

## 📝 Instrucciones

1. Implementa en `src/main.c`:
   - `void *my_memcpy(void *dest, const void *src, size_t n);`
     *Pista: castear ambos a (char *) y copiar bloque con un for/while.*
   - `void *my_memset(void *s, int c, size_t n);`
     *Pista: castear a (char *) y rellenar iterativamente el byte 'c'.*

2. En el `main()`, inicializa dos arreglos enormes de al menos 10 o 50 MegaBytes.
3. Copia el arreglo con tu lenta `my_memcpy` midiendo con `clock_gettime()`.
4. Copia del arreglo original usando la veloz `<string.h> memcpy` original y comprueba los tiempos.

## ✅ Criterios de Éxito
- Has creado código sin dependencias a `<string.h>` para las operaciones manuales.
- El benchmark arroja resultados claros demostrando el abismo de rendimiento entre un "for" ingenuo de C y el verdadero "memcpy".
