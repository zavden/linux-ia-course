# Ejercicio 2.2 — stdio vs Syscalls: El valor de los Buffers

## 🎯 Objetivo
Hacer una comparativa (benchmark) que te grabe en fuego por qué existe la librería estándar de C (`stdio.h`). Vas a comparar un programa que copia un archivo leyendo de a 1 Byte usando Syscalls, vs Leyendo de a 1 Byte usando `stdio`.

## 📚 Teoría Mínima
- Las syscalls (`read()`, `write()`) requieren hacer un **Context Switch**: tu programa Pausa su ejecución, el CPU entra en "Modo Kernel", viaja hasta el driver del disco duro, busca 1 byte, luego cambia a "Modo Usuario" y continua tu programa. Repetir esto 1 millón de veces por un archivo de 1 MB puede tardar **segundos**.
- `stdio` (`fopen`, `fgetc`, `fputc`) es un **Wrapper Inteligente**. Cuando tú le pides 1 triste byte usando `fgetc()`, el envoltorio inteligentemente hace un solo `read()` secreto por debajo de 4096 bytes (4 KB). Te entrega tu 1 byte y se guarda los 4095 restantes en un "buffer interno" en la RAM de tu proceso de usuario. Tus próximas 4095 llamadas a `fgetc` ni siquiera tocarán el Kernel, solo leerán de la RAM local al instante. 

## 📝 Instrucciones

1. Crea dos funciones separadas en `src/main.c`:
   - `void copy_syscall_1byte(const char *in, const char *out)`: Abre con `open()`, usa `read()` de tamaño 1 devolcándolo con `write()`.
   - `void copy_stdio_1byte(const char *in, const char *out)`: Abre con `fopen()`, extrae con `fgetc()` devolcándolo con `fputc()`.
2. En tu `main`, define tiempos `CLOCK_MONOTONIC`.
3. Crea un archivo grande para pruebas (Ej: genera internamente uno de 5 a 10 MB).
4. Ejecuta y temporiza ambas funciones.
5. Imprime cuántas veces más rápido fue `stdio`.

## ✅ Criterios de Éxito
- Verás con tus propios ojos que leer de a 1 Byte con `stdio` es perfectamente viable, mientras que con syscalls congela tu PC por el abrumador castigo que le haces al Kernel. 
- Te quedará clarísimo que si haces una utilidad que procesa cadenas o caracteres, `stdio` es mandatorio. Si transfieres bloques de red grandes sin parsear, `syscalls` son mejores porque te ahorras memcopys redundantes.
