# Ejercicio 1.4 — Manejo de Errores POSIX

## 🎯 Objetivo
Aprender a manejar errores de llamadas al sistema (syscalls) en C. En lenguajes modernos usamos `try/catch`. En C, usamos códigos de retorno, `errno`, y el infame pero muy útil patrón `goto cleanup`.

## 📚 Teoría Mínima
Casi todas las funciones del sistema en Linux (open, malloc, pthread, etc.) retornan un valor que indica éxito o fracaso (generalmente `-1` o `NULL`).
Cuando fallan, el Kernel de Linux setea una variable global secreta llamada `errno` (Error Number) con un código numérico (Ej: `2` = No such file or directory).

- `perror("Mensaje")`: Imprime tu mensaje seguido de la traducción en texto del último `errno`.
- `strerror(errno)`: Devuelve directamente el texto del error.

**El patrón "goto cleanup"**
En C, si abres 3 archivos y alojas memoria, pero la 4ta operación falla, tienes que cerrar todo al revés antes de retornar. Repetir código de cerrar archivos en cada if() es horrible. Usamos `goto cleanup;` para saltar al final de la función donde se hace un cleanup unificado e idempotente.

## 📝 Instrucciones

El código base en `src/main.c` intenta simular una operación de lectura de un archivo inexistente, y crear dinámicamente memoria.
Tu tarea es:
1. Crear una macro o función mágica `CHECK(llamada, mensaje)` que evalúe si el código de la "llamada" fue < 0. Si falla, debe imprimir en qué Archivo y en qué Línea (`__FILE__`, `__LINE__`) falló usando una macro de C, luego saltar al `cleanup`.
2. Implementar el patrón `goto cleanup` en el `main`. El cleanup debe liberar los punteros (si no son nulos) y cerrar el archivo (si está abierto).

## ✅ Criterios de Éxito
- Ejecutar el programa simula un error de archivo, imprime exactamente la línea del código y libera la memoria limpiamente demostrando la "excepción en C".
- Valgrind sale 100% limpio.
