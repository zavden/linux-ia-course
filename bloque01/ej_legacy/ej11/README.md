# Ejercicio 1.1 — Argumentos y `getopt`

## 🎯 Objetivo
Aprender a parsear argumentos de línea de comandos en C de forma profesional, imitando las herramientas reales de Linux usando `getopt` y `getopt_long`.

## 📚 Teoría Mínima
Los programas en C reciben argumentos en `main(int argc, char *argv[])`.
- `argc`: Argument Count (Número de argumentos, incluyendo el nombre del programa en sí).
- `argv`: Argument Vector (Arreglo de strings).

En lugar de hacer loops manuales analizando `argv[i] == "-v"`, POSIX nos entrega la función `<unistd.h> getopt()` y Linux extiende esto con `<getopt.h> getopt_long()` para soportar opciones largas como `--verbose`.

## 📝 Instrucciones

Ve a `src/main.c` e implementa lo siguiente:

1. El programa debe aceptar tres opcionales argumentos:
   - Corto `-v` / Largo `--verbose`: Imprime mensajes de debug (bandera bool).
   - Corto `-o` / Largo `--output`: Toma un argumento requerido con el nombre de un archivo.
   - Corto `-n` / Largo `--number`: Toma un argumento requerido numérico.
   - Corto `-h` / Largo `--help`: Imprime el formato de uso del programa.
2. Si se llama a `--help`, debes imprimir un mensaje estándar (como una mini man-page) explicando las opciones y salir con `EXIT_SUCCESS`.
3. Si el usuario pasa un argumento desconocido o no pasa un valor para `-o` / `-n`, el programa debe imprimir uso y salir con `EXIT_FAILURE`.
4. Si no se pasa o, y no pasa n, debe setear valores por defecto (output default: `salida.txt`, n default: `0`).
5. Finalmente, imprime las opciones leídas de forma ordenada para verificar.

## ✅ Criterios de Éxito
- Ejecutar `make test` pasará la validación si lograste hacer que reconozca tanto `-v -o miarchivo.txt -n 15` como `--verbose --output=miarchivo.txt --number 15`.
- Se requiere un correcto manejo de `getopt_long()`.
